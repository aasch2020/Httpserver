#include "request.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include "response.h"
#include <unistd.h>
#include <fcntl.h>
struct Request {
    char type[9], uri[22];
    unsigned int vernum;
    unsigned int verdec;
    char **header_key;
    char **header_vals;
    int numheads;
};
Request *request_create( char *type, char *uri, int vernum, int verdec){
  Request *r = (Request *) calloc(1, sizeof(Request));
     r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    printf("uri when given is%s\n", uri);
    strncpy(r->type, type, 8);
    strncpy(r->uri, uri, 21);
    r->vernum = vernum;
    r->verdec = verdec;

    r->numheads = 0;  
    
 r->numheads = 0;

  return r;  
}

void request_delete(Request **r) {
    for (int i = 0; i <= ((*r)->numheads); i++) {
        free((*r)->header_key[i]);
        free((*r)->header_vals[i]);
    }
    free((*r)->header_vals);
    free((*r)->header_key);
    free(*r);
}

void add_header(Request *r, char *header_total) {
    printf("%s\n", header_total);
    int len = strlen(header_total);
    printf("%s, thee length is %d\n", header_total, len);
    char *header_keyin = (char *) calloc(len + 1, sizeof(char));
    char *header_valin = (char *) calloc(len + 1, sizeof(char));
    sscanf(header_total, "%[^':']: %[^'\r']", header_keyin, header_valin);
    r->header_key[r->numheads] = header_keyin;
    r->header_vals[r->numheads] = header_valin;

    r->numheads += 1;
    if (r->numheads % 30 == 0) {
        printf("this shouldn't happen\n");
        r->header_key = realloc(r->header_key, (r->numheads + 30) * sizeof(char *));

        r->header_vals = realloc(r->header_vals, (r->numheads + 30) * sizeof(char *));
    }
}

void print_req(Request *r) {
    printf("%s, %s, HTTP/%u.%u\r\n", r->type, r->uri, r->vernum, r->verdec);
    for (int i = 0; i < r->numheads; i++) {
        printf("%s\n", r->header_key[i]);
        printf("%s\n", r->header_vals[i]);
    }
    printf("done print");
}

int validate(Request *r) {
    int regs;
    regex_t reg;
    regex_t regmethod;

    regcomp(&regmethod, "[A-Za-z]+", REG_EXTENDED);
    regcomp(&reg, "([/]([a-zA-Z0-9._]+))+", REG_EXTENDED);
    regs = regexec(&reg, r->uri, 0, NULL, 0);
    if (regs != 0) {
        regfree(&regmethod);
        regfree(&reg);

        return 1;
    }
    if (0 != regexec(&regmethod, r->type, 0, NULL, 0)) {
        regfree(&regmethod);
        regfree(&reg);

        return 1;
    }

    if (!(((strcmp(r->type, "GET") == 0) || (strcmp(r->type, "PUT") == 0))
            || (strcmp(r->type, "APPEND") == 0))) {
        regfree(&reg);
        regfree(&reg);

        return 2;
    }
    if (!((r->vernum == 1) && (r->verdec == 1))) {
        regfree(&regmethod);
        regfree(&reg);

        return 2;
    }
    regfree(&reg);
    regfree(&regmethod);

    return 0;
}

int type(Request *r) {
    if (strcmp(r->type, "GET") == 0) {
        return 1;
    }
    if (strcmp(r->type, "PUT") == 0) {
        return 2;
    }
    if (strcmp(r->type, "APPEND") == 0) {
        return 3;
    }
    return 0;
}

const char *get_uri(Request *r) {
    return r->uri;
}
int add_headderbuff(

    Request *r, char *buff, int start, int end) {
    printf("add headder from buffer\n");
    regex_t reghead;
    char *matchstr;
    int lenmatch;
    int nummatch = 0;
    int total_read = 0;
    //    int prevmatchend = start;
    ssize_t spot = 0;
    bool moreheads = true;
    regcomp(&reghead, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    regmatch_t regs;
    while (0 == regexec(&reghead, buff + start + spot, 1, &regs, REG_NOTEOL) ) {
   
        nummatch++;
        lenmatch = regs.rm_eo - regs.rm_so;
        printf(" match point is%d\n", lenmatch);
        matchstr = strndup(buff + start + spot, lenmatch);
        //      printf("%s\n", matchstr);
        add_header(r, matchstr);
        spot += lenmatch;
        free(matchstr);
        if (spot + start == end) {
            break;
            printf("%ld, %dread all\n", spot + start, end);
            moreheads = true;
        }
        if ((*(buff + start + spot) == '\r') && (*(buff + start + spot) == '\r')) {
            total_read = start + spot + 2;
            printf("breaking right");
        }
    }
    regfree(&reghead);
    if (spot + start == end) {
        return -1;
    }
    return total_read;
}

int execute_req(Request *r, int connfd) {
    int types = type(r);
    if (types == 1) {
	printf("doing a get req");
        int opened = open(r->uri, O_RDONLY);
        int resptype = 200; 
        Response *resp = response_create(resptype);
        write_file(resp, opened, connfd);
       response_delete(&resp);

    }
    if (types == 2) {
        write(connfd, "GET request\r\n", 13);
    }
    if (types == 3) {
    }
    return 0;
   
}
