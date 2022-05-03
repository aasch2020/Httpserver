#include <assert.h>
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
    char onebuff[2048] = { '\0' };
    char twobuff[2048] = { '\0' };

    while (1) {
        Request *r = request_create();
        if (hcreadstart(r, connfd, fromend, &altrend, onebuff, twobuff) == -1) {
            printf("overreadvalue = %d %s %s\n", altrend, onebuff, twobuff);
            break;
        }
        printf("overreadvalue = %d %s %s\n", altrend, onebuff, twobuff);
        if (-1 == headreadstart(r, connfd, altrend, &fromend, twobuff, onebuff)) {
            break;
        }
        printf("overreadvalue = %d %s %s\n", fromend, onebuff, twobuff);
        fflush(stdout);
        int typed = type(r);
        print_req(r);
        switch (typed) {
        case 1: execute_get(r, connfd); break;
        case 2:
            // execute_put(r, connfd);
            break;
        case 3:
            // execute_append(r, connfd);
            break;
        case 4: printf("badreq"); break;
        case 0: printf("unimp req"); break;
        }
    }
    printf("\n");
    (void) connfd;
}

/*#define BUF_SIZE 4096
void handle_connection(int connfd) {
    char buffer[BUF_SIZE] = { '\0' };
    // char bufferread[BUF_SIZE];
    //  char headbuff[BUF_SIZE];
    ssize_t bytez = 0;
    //   int processed = 0;
    //   ssize_t byteget = 0;
    //    char req[8] = {'\0'};
    char uri[22] = { '\0' };
    uri[0] = '.';
    //  int totalread;
    //   unsigned int vernum = 0, verdec = 0;
    // ssize_t readin = 0;
    int validates, types , fileop;
    // int allheads = 0;
    int proced = 0;
    int subbytes;
    /// read all bytes from connfd un:Wq
    //til we see an error or EOF
    Request *got;
    int headread = -1;
    regex_t regstatus;
    char *statmatch;
    int lenmatch = 0;
//    int headreadin = 0;
  //  int readin = 0;
    regmatch_t regmatches;
    regcomp(&regstatus, "[a-z, A-Z]{1,8}[ ]+[/][a-zA-Z0-9._]{1,18}[ ]+[H][T][T][P][/][0-9][.][0-9]",
        REG_EXTENDED);
    while ((bytez = read(connfd, buffer, BUF_SIZE)) > 0) {
        printf("loop top %s\n", buffer);

        if (0 != regexec(&regstatus, buffer, 1, &regmatches, REG_NOTEOL)) {
            Response *invresp = response_create(400);
            writeresp(invresp, connfd);
            response_delete(&invresp);
            printf("aaaaaaaaaaaaaaaaaaaaaaaaaa invalid request 1 \n");
        } else {
            lenmatch = regmatches.rm_eo - regmatches.rm_so;
                 printf("the lenmatc is%d\n", lenmatch);
            statmatch = strndup(buffer + regmatches.rm_so, lenmatch);
            proced = regmatches.rm_eo;

            printf("the match is%s proced is%d bytez is %zd\n", statmatch, proced, bytez);
                  
            if(proced == bytez){
              int inv = read_somechar(connfd, 2, "\r\n");
              if(inv == 1){
                 proced +=2;
                 
              }else{
                printf("badreq\n");
 Response *invresp = response_create(400);
            writeresp(invresp, connfd);
            response_delete(&invresp);
            
                break;
               } 
      bytez = 0;
      proced = 0;      
            }else{
             printf("the match is%s\n", statmatch);

            proced = regmatches.rm_eo + 2;
            got = request_create(statmatch);

             } 
            
            got = request_create(statmatch);
       validates = validate(got);
            if(validates == 2){
              Response *invresp = response_create(501);
              writeresp(invresp, connfd);
              response_delete(&invresp);
            
            }

            printf("procedd %d bytez %zd\n", proced, bytez);
            headread = add_headderbuff(got, buffer, proced, bytez, &proced);
            printf("the header of read is the %d\n", headread);
            proced = headread;
            while (headread == -1) {
                printf("in the header loop\n");
                while ((headread == -1) && ((subbytes = read(connfd, buffer, BUF_SIZE)) > 0)) {
                    printf("reading extra\n");
                    headread = add_headderbuff(got, buffer, 0, subbytes, &proced);
                }
            }
            printf("porocessed after heads %d, all after %zd\n", proced, bytez);
            print_req(got);

           

            types = type(got);
            printf("printing the request\n\n\n");
            if(types == 1){
                execute_get(got, connfd);
                printf("executed the get request\n");
             }else if(types == 2){
                  execute_put(got, connfd, buffer, proced, bytez);
               }else if(types == 3){

     execute_append(got, connfd, buffer, proced, bytez);

}
              
            print_req(got);
       
            memset(buffer, 0, BUF_SIZE);
            request_delete(&got);
        }
    }

    printf("broke the while\n");
    (void) connfd;
}*/
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
