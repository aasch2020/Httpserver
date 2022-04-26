#include "request.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>


struct Request {
    char type[9], uri[22];
    unsigned int vernum;
    unsigned int verdec;
    char **header_key;
    char **header_vals;
    int numheads;
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
      return r;
}

void request_delete(Request **r) {
    for (int i = 0; i < ((*r)->numheads); i++) {
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
    printf("%s, length is %d\n", header_total, len);
    char* header_keyin = (char*)calloc(len, sizeof(char)); 
    char* header_valin =  (char*)calloc(len, sizeof(char));  
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
    regex_t reghead;
    char* matchstr;
    int lenmatch;
    int nummatch = 0;
    int total_read = 0;
//    int prevmatchend = start;
    ssize_t spot = 0;
    regcomp(&reghead, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    regmatch_t regs[1];
    while(0 == regexec(&reghead, buff + start+spot, 1, regs, REG_NOTEOL)){
       nummatch++;
       lenmatch = regs->rm_eo - regs->rm_so;
//       printf("%d\n", lenmatch);
       matchstr = strndup(buff+start+spot, lenmatch);
 //      printf("%s\n", matchstr);
       add_header(r, matchstr);
       spot += lenmatch;
       free(matchstr);
       if(spot+start == end){
         break;
       }
       if((*(buff+start+spot)=='\r')&& (*(buff+start+spot)=='\r')){
          total_read =  start+spot+2;
	  printf("breaking right");

       }
       
    }
    regfree(&reghead);
    return total_read;
  
}

/*int execute(Request *r){
  int types = type(r);
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
  if(types == 2){
  }
  if(types == 3){
  
  }

}*/
