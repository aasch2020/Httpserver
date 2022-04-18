#include "request.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
struct Request {
  char type[8], uri[21];
  unsigned int vernum;
  unsigned int verdec;
  char** header_fields;
  int numheads;
  regex_t reg;
};

Request *request_create(char *type, char *uri, int vernum, int verdec){
  Request *r = (Request*)calloc(1, sizeof(Request));
  strcpy(r->type, type);
  strcpy(r->uri, uri);
  r->vernum = vernum;
  r->verdec = verdec;
  r->numheads = 0;
  r->header_fields = (char**)calloc(30, sizeof(char*));
 regcomp(&(r->reg), "([/]([a-zA-Z0-9._]+))+", REG_EXTENDED);

  return r;
}

void request_delete(Request **r){
  for(int i = 0; i < ((*r)->numheads); i++){
    free((*r)->header_fields[i]);

  }
  free((*r)->header_fields);
  regfree(&(*r)->reg);
  free(*r);

}

void add_header(Request *r, char *header){
  int lenstr = strlen(header);
  r->header_fields[r->numheads] = (char*)calloc(lenstr, sizeof(char*));
  r->numheads++;
  if(r->numheads % 30 == 0){
	  r->header_fields = realloc(r->header_fields, (r->numheads+30)*sizeof(char*));
  }

}

void print_req(Request *r){
  printf("%s, %s, HTTP/%u.%u\r\n", r->type, r->uri, r->vernum, r->verdec);
  for(int i = 0; i < r->numheads; i++){
    printf("%s\r\n", r->header_fields[i]);
  }

}

int validate(Request *r){
  int regs;
    regs = regexec(&(r->reg), r->uri, 0, NULL, 0);
     if(regs != 0){
       return 1;
     }else{
       return 0;
     }

  
}

int execute(Request *r){
  if(strcmp(r->type, "GET") == 0){
     
  }


}
