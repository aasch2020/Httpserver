#include "request.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
struct Request {
    char type[8], uri[21];
    unsigned int vernum;
    unsigned int verdec;
    char **header_key;
    char **header_vals;
    int numheads;
    regex_t reg;
};

Request *request_create(char *type, char *uri, int vernum, int verdec) {
    Request *r = (Request *) calloc(1, sizeof(Request));
    strcpy(r->type, type);
    strcpy(r->uri, uri);
    r->vernum = vernum;
    r->verdec = verdec;
    r->numheads = 0;
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));

    regcomp(&(r->reg), "([/]([a-zA-Z0-9._]+))+", REG_EXTENDED);

    return r;
}

void request_delete(Request **r) {
    for (int i = 0; i < ((*r)->numheads); i++) {
        free((*r)->header_key[i]);
        free((*r)->header_vals[i]);
    }
    free((*r)->header_vals);
    free((*r)->header_key);

    regfree(&(*r)->reg);
    free(*r);
}

void add_header(Request *r, char *header_keyin, char *header_valin) {
    int lenstr = strlen(header_keyin);
    printf("%s\n", header_keyin);
    printf("%s\n", header_valin);
    printf("%d\n", r->numheads);
    int lenval = strlen(header_valin);
    r->header_key[r->numheads] = (char *) calloc(lenstr, sizeof(char));
    r->header_vals[r->numheads] = (char *) calloc(lenval, sizeof(char));
    strcpy(r->header_key[r->numheads], header_keyin);
    strcpy(r->header_vals[r->numheads], header_valin);
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
        printf("%s\n", r->header_vals[i]);
        printf("%s\n", r->header_key[i]);
    }
}

int validate(Request *r) {
    int regs;
    regs = regexec(&(r->reg), r->uri, 0, NULL, 0);
    if (regs != 0) {
        return 1;
    } else {
        return 0;
    }
}

//int execute(Request *r){
//  if(strcmp(r->type, "GET") == 0){

//  }

//}
