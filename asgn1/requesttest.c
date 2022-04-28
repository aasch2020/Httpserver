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
#define BUF_SIZE 4096
int main(){
   //   int processed = 0;
    //   ssize_t byteget = 0;
    Request *test = request_create("GET", "/foo.txt", 1, 1);
    char requests[]= "foo: bar\r\nfoos: baas\r\nno: wa\r\nlast: one\r\nContent-Length: 12\r\n\r\ndont: match";
    int total = add_headderbuff(test, requests, 0, strlen(requests));
    print_req(test); 
    printf("%s", requests+total);
    request_delete(&test);
  return 0;
}


