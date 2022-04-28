#include "request.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/stat.h>
struct Response {
    char statphrase[25];
    char **header_key;
    char **header_vals;
    char *msgbody;
    int numheads;
    unsigned int bodylen;
    struct stat *filestat;
}

Response *
    response_create(int type) {
    Response *r = (Response *) calloc(1, sizeof(Request));
    filestat = malloc(sizeof(struct stat));
    switch (type) {

    case 200: r->statphrase = "OK"; r->msgbody = (char *) calloc(3, sizeof(char));

    case 201:
        r->statphrase = "Created";
        r->msgbody = (char *) calloc(10, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 400:
        r->statphrase = "Bad Request";
        r->msgbody = (char *) calloc(11, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 403:
        r->statphrase = "Forbidden";
        r->msgbody = (char *) calloc(13, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 404:
        r->statphrase = "Not Found";
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 500:
        r->statphrase = "Internal Server Error";
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    case 501:
        r->statphrase = "Not Implemented";
        r->msgbody = (char *) calloc(20, sizeof(char));
        strcpy(r->msgbody, "OK\n");
    }
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    r->numheads = 0;
}

void addheaderres(Request *r, char *header_keyin, char *header_valin) {
    printf("%s\n", header_keyin);
    printf("%s\n", header_valin);
    printf("%d\n", r->numheads);
    strndup(r->header_key[r->numheads], header_keyin);
    strndup(r->header_vals[r->numheads], header_valin);
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

void write_nofile(Response *r, int connec){
}

void write_file(Response *r, int filewrt, int connec) {
    stat(filewrt, r->filestat);
    char writebuf[2048];
    bodylen += r->filestat->off_t;
    char wrtstr = wrt[32];
    addheaderres(r, "Content-Length", itoa(bodyleni, 10));
    unsigned int written = 0;
    int readin = 0;
    while (written != bodylen) {
        readin = read(filewrt, writebuf, 2048);
        written += write(connec, writebuf, readin);
    }
}
