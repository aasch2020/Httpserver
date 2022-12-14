#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include "request.h"
#include "response.h"
#include "queue.h"
#include <stdbool.h>
#define OPTIONS              "t:l:"
#define BUF_SIZE             4096
#define DEFAULT_THREAD_COUNT 4

static FILE *logfile;
#define LOG(...) fprintf(logfile, __VA_ARGS__);
pthread_t *threadq;
int threads = DEFAULT_THREAD_COUNT;
bool stop = false;
void sighand() {
    fflush(logfile);
    fclose(logfile);
    for (int i = 0; i < threads; i++) {
        pthread_cancel(threadq[i]);
        if (errno == EINVAL) {
            printf("invalid\n");
        }
    }

    exit(1);
}

// Converts a string to an 16 bits unsigned integer.
// Returns 0 if the string is malformed or out of the range.
static size_t strtouint16(char number[]) {
    char *last;
    long num = strtol(number, &last, 10);
    if (num <= 0 || num > UINT16_MAX || *last != '\0') {
        return 0;
    }
    return num;
}

// Creates a socket for listening for connections.
// Closes the program and prints an error message on error.
static int create_listen_socket(uint16_t port) {
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
    if (listen(listenfd, 128) < 0) {
        err(EXIT_FAILURE, "listen error");
    }
    return listenfd;
}
static void handle_connection(int connfd) {
    struct Request r;
    int fromend = 0;
    int altrend = 0;
    int chekr = 0;
    char onebuff[2048] = { '\0' };
    char twobuff[2048] = { '\0' };
    bool severerr = false;
    bool badreq = false;
    request_init(&r);
    chekr = 0;
    severerr = false;
    badreq = false;
    altrend = 0;
    fromend = 0;
    if (hcreadstart(&r, connfd, fromend, &altrend, onebuff, twobuff) == -1) {
        severerr = true;
    }
    if (-1 == headreadstart(&r, connfd, altrend, &fromend, twobuff, onebuff)) {
        severerr = true;
    }
    int typed = type(&r);
    switch (typed) {
    case 1: execute_get(&r, connfd, logfile); break;
    case 3:
        if ((chekr = execute_append(&r, connfd, onebuff, &altrend, twobuff, fromend, logfile))
            == 1) {
            fromend = 0;
            altrend = 0;
            badreq = true;
            memset(onebuff, '\0', 2048);
            memset(twobuff, '\0', 2048);
        }

        break;
    case 2:
        if ((chekr = execute_put(&r, connfd, onebuff, &altrend, twobuff, fromend, logfile)) == 1) {
            fromend = 0;
            altrend = 0;
            badreq = true;
            memset(onebuff, '\0', 2048);
            memset(twobuff, '\0', 2048);
        }
        break;
    case 4:
        fromend = 0;
        Response *resp = response_create(400);
        writeresp(resp, connfd);
        response_delete(&resp);
        altrend = 0;

        badreq = true;
        memset(onebuff, '\0', 2048);
        memset(twobuff, '\0', 2048);

        fflush(stdout);
        break;
    case 0:
        fromend = 0;
        Response *respun = response_create(501);
        writeresp(respun, connfd);
        response_delete(&respun);

        altrend = 0;
        badreq = true;
        memset(onebuff, '\0', 2048);
        memset(twobuff, '\0', 2048);
        break;
    }
    request_clear(&r);
    if (chekr == -1) {
        Response *respun = response_create(500);
        writeresp(respun, connfd);
        response_delete(&respun);
        fromend = 0;
        altrend = 0;
        badreq = true;
        memset(onebuff, '\0', 2048);
        memset(twobuff, '\0', 2048);
    }
    close(connfd);
    return;
}
void sigterm_handler() {
    for (int i = 0; i < threads; i++) {

        pthread_cancel(threadq[i]);
    }
    fflush(logfile);
    fclose(logfile);
    exit(EXIT_SUCCESS);
}

static void usage(char *exec) {
    fprintf(stderr, "usage: %s [-t threads] [-l logfile] <port>\n", exec);
}
void executeConn(int theconnfd) {
    handle_connection(theconnfd);
    return;
}
Queue jobqueue;
pthread_mutex_t buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

pthread_cond_t emptied = PTHREAD_COND_INITIALIZER;
void *threadjob() {
    int theconnfd = -1;
    while (1) {
        pthread_mutex_lock(&qlock);
        while (empty(&jobqueue)) {
            pthread_cond_wait(&emptied, &qlock);
        }

        theconnfd = deQueue(&jobqueue);
        pthread_cond_signal(&fill);

        pthread_mutex_unlock(&qlock);
        executeConn(theconnfd);
    }
    return NULL;
}

void produceconnfd(int connfd) {
    pthread_mutex_lock(&qlock);
    while (full(&jobqueue)) {
        pthread_cond_wait(&fill, &qlock);
    }
    enQueue(&jobqueue, connfd);
    pthread_cond_signal(&emptied);

    pthread_mutex_unlock(&qlock);
}
int main(int argc, char *argv[]) {
    int opt = 0;
    logfile = stderr;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't':
            threads = strtol(optarg, NULL, 10);
            if (threads <= 0) {
                errx(EXIT_FAILURE, "bad number of threads");
            }
            break;
        case 'l':
            logfile = fopen(optarg, "w");
            if (!logfile) {
                errx(EXIT_FAILURE, "bad logfile");
            }
            break;
        default: usage(argv[0]); return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        warnx("wrong number of arguments");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    uint16_t port = strtouint16(argv[optind]);
    if (port == 0) {
        errx(EXIT_FAILURE, "bad port number: %s", argv[1]);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sighand);
    signal(SIGTERM, sigterm_handler);

    threadq = (pthread_t *) calloc(threads, sizeof(pthread_t));
    int listenfd = create_listen_socket(port);
    init_queue(&jobqueue);
    for (int i = 0; i < threads; i++) {
        pthread_create(&threadq[i], NULL, threadjob, NULL);
    }
    for (;;) {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            warn("accept error");
            continue;
        }
        produceconnfd(connfd);
    }

    return EXIT_SUCCESS;
}
