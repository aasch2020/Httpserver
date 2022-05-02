#include "request.h"
#include <stdlib.h>
#include <errno.h>
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
    int content_len;
};
Request *request_create(char *match) {
    Request *r = (Request *) calloc(1, sizeof(Request));
    r->uri[0] = '.';
    sscanf(match, "%8[a-zA-Z] %22s HTTP/%u.%u", r->type, r->uri + 1, &(r->vernum), &(r->verdec));

    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    r->numheads = 0;
    r->content_len = -1;
    r->numheads = 0;
    print_req(r);
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
    printf("addin the header from%s\n", header_total);
    int len = strlen(header_total);
    printf("%s, thee length is %d\n", header_total, len);
    
    char *header_keyin = (char *) calloc(len + 1, sizeof(char));
    char *header_valin = (char *) calloc(len + 1, sizeof(char));
    sscanf(header_total, "%[^':']: %[^'\r']", header_keyin, header_valin);
    r->header_key[r->numheads] = header_keyin;
    r->header_vals[r->numheads] = header_valin;
    if(strcmp(header_keyin, "Content-Length") == 0){
      r->content_len = atoi(header_valin);
    }
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

    Request *r, char *buff, int start, int end, int *proced) {
    printf("add headder from buffer\n");
    regex_t reghead;
    if (start == end) {
        printf("need read more\n");
        return -1;
    }
    int nummatch = 0;
    char *matchstr;
    int lenmatch = 0;
      int total_read = 0;
    //    int prevmatchend = start;
    ssize_t spot = 0;
    bool moreheads = false;
    regcomp(&reghead, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    regmatch_t regs;
    while (0 == regexec(&reghead, buff + start + spot, 1, &regs, REG_NOTEOL)) {

        nummatch++;
        lenmatch = regs.rm_eo - regs.rm_so;
        *proced += lenmatch;
        printf(" match point is%d\n", lenmatch);
        matchstr = strndup(buff + start + spot, lenmatch);
        //      printf("%s\n", matchstr);
        printf("the end is,%ld the last is,%d\n", spot + start, end);
        spot += lenmatch;
        printf(
            "\n\nwhaoo the thing at the end the thingy whoa is\n\n %s\n\n\n", buff + start + spot);
        add_header(r, matchstr);

        free(matchstr);
        printf("the spot and start is this%ld, %dread all\n", spot + start, end);

        if (spot + start == end) {
            break;
            printf("the spot and start is this%ld, %dread all\n", spot + start, end);
            moreheads = true;
        }
        if ((*(buff + start + spot) == '\r') && (*(buff + start + spot) == '\r')) {
            total_read = start + spot + 2;
            printf("breaking right\n");
            moreheads = false;
        }
    }
    regfree(&reghead);
    if (moreheads) {
        printf("\n\nreturnng to read more\n\n");
        return -1;
    }else{
        printf("\n\ndone reading\n\n");
        return total_read;
    }
}

int execute_get(Request *r, int connfd) {
    int types = type(r);
    if (types == 1) {
        printf("doing a get req");
        int opened = open(r->uri, O_RDONLY);
        //     int error = errno;
        if (opened == -1) {
            if (errno == EACCES) {
                Response *errrep = response_create(403);
                writeresp(errrep, connfd);
                return 1;
            }
            if (errno == ENOENT) {
                Response *errrep = response_create(404);
                writeresp(errrep, connfd);
                return 1;
            }
        }
        int resptype = 200;
        Response *resp = response_create(resptype);
        write_file(resp, opened, connfd);
        response_delete(&resp);
    }
    if (types == 2) {
         if (types == 3) {
       write(connfd, "Append request\r\n", 20); 
}
    return 0;
}return 0;
}
int execute_append(Request *r, int connfd, char* buffer, int start, int end){
   printf( "PUT request\n");
        bool created = true;
        int opened = open(r->uri, O_WRONLY|O_APPEND);
        if(opened == -1){
           printf("not openend right\n");
           if (errno == ENOENT) {
                   printf("bad access somehow\n");
                Response *errrep = response_create(404);
                writeresp(errrep, connfd);
                return 1;
     }
           if (errno == EACCES) {
                printf("bad access somehow\n");
                Response *errrep = response_create(403);
                writeresp(errrep, connfd);
                return 1;
            }             
        }
          bool readfrombuf = false;
          if(start != end){
             readfrombuf = true;
          }
          
          int remain  = end - start;
          printf("the buff %d the write%d\n", start - end, r->content_len);
          if(remain >= r->content_len){
            printf("written loop 1\n");
            write(opened, buffer + start, r->content_len);
               
          }else{
          if(end - start != 0){
             write(opened, buffer + start, end - start); 
          } 
          int writed = end - start;
          
          char bufftwo[2048];
          int readed;
          while(writed < r->content_len){ 
         //  printf("in the write loop written %d, need to writed %d\n");
           readed = read(connfd, bufftwo, 2048);
            write(opened, bufftwo, readed);
            printf("%s", bufftwo);
            writed+=readed;
         
         }
       } 
        
         if(!created){
Response *resp = response_create(201);
           writeresp(resp, connfd);
        }else{
         Response *resp = response_create(200);
           writeresp(resp, connfd);
 
         }
    
 return 0;   
}

int execute_put(Request *r, int connfd, char* buffer, int start, int end){
   printf( "PUT request\n");
        bool created = true;
        int opened = open(r->uri, O_CREAT|O_EXCL|O_WRONLY, 0666);
        if(opened == -1){
           printf("not openend right\n");
           if (errno == EEXIST) {
             printf("the swag thing\n");
             created = false; 
             opened = open(r->uri, O_WRONLY);
             if(opened == -1){
              if (errno == EACCES) {
                Response *errrep = response_create(403);
                writeresp(errrep, connfd);
                return 1;
              }
  
            }
           }
           if (errno == EACCES) {
                printf("bad access somehow\n");
                Response *errrep = response_create(403);
                writeresp(errrep, connfd);
                return 1;
            }             
        }
          bool readfrombuf = false;
          if(start != end){
             readfrombuf = true;
          }
          
          int remain  = end - start;
          printf("the buff %d the write%d\n", start - end, r->content_len);
          if(remain >= r->content_len){
            printf("written loop 1\n");
            write(opened, buffer + start, r->content_len);
               
          }else{
          if(end - start != 0){
             write(opened, buffer + start, end - start); 
          } 
          int writed = end - start;
          
          char bufftwo[2048];
          int readed;
          while(writed < r->content_len){ 
         //  printf("in the write loop written %d, need to writed %d\n");
           readed = read(connfd, bufftwo, 2048);
            write(opened, bufftwo, readed);
            printf("%s", bufftwo);
            writed+=readed;
         
         }
       } 
        
        Response *resp = response_create(200);
           writeresp(resp, connfd);
 
         
    
 return 0;   
}

