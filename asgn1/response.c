#include "request.h"
#include "response.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>
struct Response {
    char statphrase[25];
    char **header_key;
    char **header_vals;
    char *msgbody;
    int numheads;
    unsigned int bodylen;
   };

Response *
    response_create(int type) {
    Response *r = (Response *) calloc(1, sizeof(Response));
       switch (type) {

    case 200: strcpy(r->statphrase, "OK"); r->msgbody = (char *) calloc(3, sizeof(char));

    case 201:
        strcpy(r->statphrase,"Created");
        r->msgbody = (char *) calloc(10, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 400:
        strcpy(r->statphrase, "Bad Request");
        r->msgbody = (char *) calloc(11, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 403:
        strcpy(r->statphrase, "Forbidden");
        r->msgbody = (char *) calloc(13, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 404:
        strcpy(r->statphrase, "Not Found");
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 500:
        strcpy(r->statphrase,"Internal Server Error");
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 501:
        strcpy(r->statphrase, "Not Implemented");
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    }
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    r->numheads = 0;
    return r;
}

void addheaderres(Response *r, char *header_keyin, char *header_valin) {
    printf("%s\n", header_keyin);
    printf("%s\n", header_valin);
    printf("%d\n", r->numheads);
    r->header_key[r->numheads] = strdup( header_keyin );
    r->header_vals[r->numheads] =  strdup(header_valin);
    r->numheads += 1;
    if (r->numheads % 30 == 0) {
        printf("this shouldn't happen\n");
        r->header_key = realloc(r->header_key, (r->numheads + 30) * sizeof(char *));

        r->header_vals = realloc(r->header_vals, (r->numheads + 30) * sizeof(char *));
    }
}
void response_delete(Response **r) {
    for (int i = 0; i < ((*r)->numheads); i++) {
        free((*r)->header_key[i]);
        free((*r)->header_vals[i]);
    }
    free((*r)->msgbody);
    free((*r)->header_vals);
    free((*r)->header_key);
    free(*r);
}

void write_file(Response *r, int filewrt, int connec) {
     struct stat filestat;
    	fstat(filewrt, &filestat);
    char writebuf[2048];
    int bodylen = 0;
    bodylen += filestat.st_size;
//    tathar wrtstr[32];
    int check = bodylen;
    int lengthofstrint = 0;
    while(check > 0){
      check = check / 10;
      lengthofstrint++;
    }
    char* thebody = (char*)calloc(lengthofstrint, sizeof(char));
    sprintf(thebody, "%d", bodylen);
    addheaderres(r, "Content-Length", thebody);
    int written = 0;
    int readin = 0;
    while (written != bodylen) {
        readin = read(filewrt, writebuf, 2048);
        written += write(connec, writebuf, readin);
    }
}
