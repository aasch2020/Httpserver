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

#define BUF_SIZE 4096
void handle_connection(int connfd) {
    char buffer[BUF_SIZE];
    char bufferread[BUF_SIZE];
    char headbuff[BUF_SIZE];
    ssize_t bytez = 0;
 //   int processed = 0;
    ssize_t byteget = 0;
    char req[8], uri[22];
    uri[0] = '.';
    char header_key[2048], header_val[2048];
    unsigned int vernum, verdec;
    ssize_t readin = 0;
    int validates, types, fileop;
    int allheads = 0;
    /// read all bytes from connfd un:Wq
    //til we see an error or EOF
   
    while ((bytez = read(connfd, buffer, BUF_SIZE)) > 0) {
      printf("loop top\n");

        if (4 != sscanf(buffer, "%8[a-zA-Z] %20s HTTP/%u.%u", req, uri + 1, &vernum, &verdec)) {
            printf("invalid request 1 \n");
        } else {
            Request *got = request_create(req, uri, vernum, verdec);
            while(!allheads){
            if(bytez == (long)(strlen(req) + strlen(uri) + 11 + readin)){
	      read(connfd, headbuff, BUF_SIZE);
	    }
            while (2
                   == sscanf(headbuff+readin,
                       "%2048[^':']: %s\r\n", header_key, header_val)) {
                printf("do the thing\n");
                readin += strlen(header_key) + strlen(header_val) + 4;
		printf("why");
                //  printf("remaining\n %s buffer\n", buffer+(strlen(req)+strlen(uri) +12+ readin));
                add_header(got, header_key, header_val);
                	
            }
            allheads=1;
            printf("stuck here\n");
	    }
	    validates = validate(got);
	   
	    types = type(got);
	    if(types == 1){
	      write(connfd, "GET request\r\n", 13);
	      printf("URI = %s\n", get_uri(got));
	      fileop = open(get_uri(got), O_RDONLY);
	      int therr = errno;
	      if(fileop == -1){
	         if(therr == 2){
		    printf("no file 404");

		 }
		 if(therr == 13){
		   printf("no perm");
		 }
	      
	      }
	      while((byteget = read(fileop, bufferread, BUF_SIZE)) > 0){
		 printf("doing the writeaaa\n");
	         write(connfd, bufferread, byteget);
	       
	      }
	    }
             
            print_req(got);
        }
    memset(buffer, 0, BUF_SIZE);
    }

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
