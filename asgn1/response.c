#include "request.h"
#include "response.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>
struct Response {
    char statphrase[31];
    char **header_key;
    char **header_vals;
    char msgbody[30];
    int numheads;
    int type;
    char httpver[9];
    unsigned int bodylen;
};

Response *response_create(int type) {
    Response *r = (Response *) calloc(1, sizeof(Response));
    r->type = type;
    strcpy(r->httpver, "HTTP/1.1");
    r->numheads = 0;
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));

    switch (type) {

    case 200: strcpy(r->statphrase, "OK\r\n"); break;
    case 201:
        strcpy(r->statphrase, "Created\r\n");
        addheaderres(r, "Content-Length", "8");
        strcpy(r->msgbody, "Created\n");
        break;
    case 400:
        strcpy(r->statphrase, "Bad Request\r\n");
        addheaderres(r, "Content-Length", "12");

        strcpy(r->msgbody, "Bad Request\n");
        break;
    case 403:
        strcpy(r->statphrase, "Forbidden\r\n");
        addheaderres(r, "Content-Length", "10");
        printf("makin a forbiden");
        strcpy(r->msgbody, "Forbidden\n");
        break;
    case 404:
        strcpy(r->statphrase, "Not Found\r\n");
        printf("makin a 404\n");
        addheaderres(r, "Content-Length", "10");
        strcpy(r->msgbody, "Not Found\n");
        break;
    case 500:
        strcpy(r->statphrase, "Internal Server Error\r\n");
        addheaderres(r, "Content-Length", "22");

        strcpy(r->msgbody, "Internal Server Error\n");
        break;
    case 501:
        strcpy(r->statphrase, "Not Implemented\r\n");
        addheaderres(r, "Content-Length", "20");

        strcpy(r->msgbody, "Not Implemented\n");
        break;
    }
    return r;
}

void addheaderres(Response *r, char *header_keyin, char *header_valin) {
    r->header_key[r->numheads] = strdup(header_keyin);
    r->header_vals[r->numheads] = strdup(header_valin);
    r->numheads += 1;
    if (r->numheads % 30 == 0) {
        r->header_key = realloc(r->header_key, (r->numheads + 30) * sizeof(char *));

        r->header_vals = realloc(r->header_vals, (r->numheads + 30) * sizeof(char *));
    }
}
void response_delete(Response **r) {
    for (int i = 0; i <= ((*r)->numheads); i++) {
        free((*r)->header_key[i]);
        free((*r)->header_vals[i]);
    }
    free((*r)->header_vals);
    free((*r)->header_key);
    free(*r);
}

void writeresp(Response *r, int connec) {
    int len = strlen(r->httpver) + strlen(r->statphrase) + 5;
    char *writebuf = (char *) calloc(len + 1, sizeof(char));
    sprintf(writebuf, "%s %d %s", r->httpver, r->type, r->statphrase);
    write(connec, writebuf, len);
    if ((r->type == 200) && (r->numheads == 0)) {
        addheaderres(r, "Content-Length", "3");
        strcpy(r->msgbody, "Ok\n");
    }
    for (int i = 0; i < r->numheads; i++) {
        write(connec, r->header_key[i], strlen(r->header_key[i]));
        write(connec, ": ", 2);
        write(connec, r->header_vals[i], strlen(r->header_vals[i]));
    }

    write(connec, "\r\n\r\n", 4);
    write(connec, r->msgbody, strlen(r->msgbody));
    free(writebuf);
}

void write_file(Response *r, int filewrt, int connec) {
    struct stat filestat;
    fstat(filewrt, &filestat);
    char writebuf[2048];
    ssize_t bodylen = 0;
    bodylen += filestat.st_size;
    int check = bodylen;
    int lengthofstrint = 0;
    while (check > 0) {
        check = check / 10;
        lengthofstrint++;
    }
    char *thebody = (char *) calloc(lengthofstrint + 1, sizeof(char));
    sprintf(thebody, "%zd", bodylen);
    addheaderres(r, "Content-Length", thebody);
    writeresp(r, connec);
    int written = 0;
    int readin = 0;
    while (written != bodylen) {
        readin = read(filewrt, writebuf, 2048);
        if (readin == 0) {
            free(thebody);

            return;
        }

        written += write(connec, writebuf, readin);
    }
    free(thebody);
}
