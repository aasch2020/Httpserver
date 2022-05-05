#include <assert.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "request.h"
#include "response.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
/**
   Converts a string to an 16 bits unsigned integer.
   Returns 0 if the string is malformed or out of the range.
 */

Request *r;

void sighand() {
    request_delete(&r);
    exit(1);
}
uint16_t strtouint16(char number[]) {
    char *last;
    long num = strtol(number, &last, 10);
    if (num <= 0 || num > UINT16_MAX || *last != '\0') {
        return 0;
    }
    return num;
}
/**
   Creates a socket for listening for connections.
   Closes the program and prints an error message on error.
 */
int create_listen_socket(uint16_t port) {
    struct sockaddr_in addr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err(EXIT_FAILURE, "socket error");
    }
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *) &addr, sizeof addr) < 0) {
        err(EXIT_FAILURE, "bind error");
    }
    if (listen(listenfd, 500) < 0) {
        err(EXIT_FAILURE, "listen error");
    }
    return listenfd;
}

int read_somechar(int connfd, int numtoread, char *check) {
    int readed = 0;
    char *buffer = (char *) calloc(numtoread, sizeof(char));
    while (readed < numtoread) {
        readed += read(connfd, buffer + readed, numtoread - readed);
    }
    printf("%s\n", buffer);
    if (0 == strcmp(buffer, check)) {
        printf("checked to 1");
        free(buffer);
        return 1;
    } else {
        free(buffer);
        return 0;
    }
}
void handle_connection(int connfd) {
    int fromend = 0;
    int altrend = 0;
   int chekr = 0;
    char onebuff[2048] = { '\0' };
    char twobuff[2048] = { '\0' };
    while (1) {
        chekr = 0;
        r = request_create();
 bool severerr = false;
        bool badreq = false;
        printf("overreadvalue = %d %s %s\n", altrend, onebuff, twobuff);

        if (hcreadstart(r, connfd, fromend, &altrend, onebuff, twobuff) == -1) {
            printf("overreadvalue = %d %s %s\n", altrend, onebuff, twobuff);
            severerr = true;
            break;
        }
        printf("overreadvalue = %d %s %s\n", altrend, onebuff, twobuff);
        if (-1 == headreadstart(r, connfd, altrend, &fromend, twobuff, onebuff)) {
            severerr = true;
            break;
        }
        printf("overreadvalue the second = %d %s %s\n", fromend, onebuff, twobuff);
        fflush(stdout);
        int typed = type(r);
        print_req(r);
        switch (typed) {
        case 1: execute_get(r, connfd); break;
        case 3:
            if ((chekr = execute_append(r, connfd, onebuff, &altrend, twobuff, fromend))== 1) {
                fromend = 0;
                altrend = 0;
                badreq = true;
                memset(onebuff, '\0', 2048);
                memset(twobuff, '\0', 2048);
            }

            break;
        case 2:
            if ((chekr = execute_put(r, connfd, onebuff, &altrend, twobuff, fromend))== 1) {
                fromend = 0;
                altrend = 0;
                badreq = true;
                memset(onebuff, '\0', 2048);
                memset(twobuff, '\0', 2048);
            }
            break;
        case 4:
            printf("badreq");
            Response *resp = response_create(400);
            writeresp(resp, connfd);
            response_delete(&resp);
            fromend = 0;
            altrend = 0;
            badreq = true;
            memset(onebuff, '\0', 2048);
            memset(twobuff, '\0', 2048);

            fflush(stdout);
            break;
        case 0:
            printf("unimp req");
            Response *respun = response_create(501);
            writeresp(respun, connfd);
            response_delete(&respun);
            fromend = 0;
            altrend = 0;
            badreq = true;
            memset(onebuff, '\0', 2048);
            memset(twobuff, '\0', 2048);

            break;
        }
        request_clear(r);
        // request_delete(&r);
        if(chekr == -1){
            Response *respun = response_create(500);
            writeresp(respun, connfd);
            response_delete(&respun);
            fromend = 0;
            altrend = 0;
            badreq = true;
            memset(onebuff, '\0', 2048);
            memset(twobuff, '\0', 2048);


break;
        }
        if (badreq) {
            break;
        }
    }
    printf("\n");
    (void) connfd;
}

int main(int argc, char *argv[]) {
    int listenfd;
    uint16_t port;
    if (argc != 2) {
        errx(EXIT_FAILURE, "wrong arguments: %s port_num", argv[0]);
    }
    port = strtouint16(argv[1]);
    if (port == 0) {
        errx(EXIT_FAILURE, "invalid port number: %s", argv[1]);
    }
    listenfd = create_listen_socket(port);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighand);
    while (1) {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            warn("accept error");
            continue;
        }

        handle_connection(connfd);
        // good code opens and closes objects in the same context. *sigh*
        close(connfd);
    }

    return EXIT_SUCCESS;
}
