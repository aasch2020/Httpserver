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
    bool badreq;
    //    char* thetotalhead;
};
Request *request_create() {
    Request *r = (Request *) calloc(1, sizeof(Request));
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    r->numheads = 0;
    r->content_len = -1;
    r->numheads = 0;
    r->badreq = false;
    return r;
}
void request_badflag(Request *r) {
    r->badreq = true;
}
void request_update(Request *r, char *match) {
    r->uri[0] = '.';
    //  r->thetotalhead = strndup(match, 40);

    sscanf(match, "%8[a-zA-Z] %22s HTTP/%u.%u", r->type, r->uri + 1, &(r->vernum), &(r->verdec));

    print_req(r);
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
int hcreadstart(
    Request *r, int connfd, int inbufsize, int *fromend, char *inbuffer, char *outbuffer) {
    int toread = 39;
    regex_t regm;
    regex_t done;
    regcomp(&regm,
        "[a-z, A-Z]{1,8}[ ][/][a-zA-Z0-9._]{1,19}[ ][H][T][T][P][/][0-9][.][0-9][\r][\n]",
        REG_EXTENDED);
    regcomp(&done, "[\r][\n]", REG_EXTENDED | REG_NOSUB);
    regmatch_t regs;
    regmatch_t enma;
    char readbuff[40] = {'\0'};
    strncpy(readbuff, inbuffer, 40);
    int readed = inbufsize;
    int readcur = 0;
    bool found = false;
    //    int endofmatch = 0;
    char *statmatch;
    int lenmatch = 0;
//    int startofmatch = 0;

    while (readed < toread) {
        readcur = read(connfd, readbuff + readed, toread - readed);
        if (readcur == 0) {
            regfree(&regm);
            printf("path 1\n");
            return -1;
        }
        printf("thiswhile\n");
        readed += readcur;
         printf("readbuff %s", readbuff);
        if (0 == regexec(&regm, readbuff, 1, &regs, 0)) {
            printf("wematchedstat\n");
            lenmatch = regs.rm_eo - regs.rm_so;
            printf("%d to here %dwematched\n", regs.rm_so, regs.rm_eo);
         //   startofmatch = regs.rm_eo - regs.rm_so;
            statmatch = strndup(readbuff + regs.rm_so, lenmatch);
            printf("the string we matched is%s\n", statmatch);
            request_update(r, statmatch);
            if (regs.rm_eo != readed) {
                strncpy(outbuffer, readbuff + regs.rm_eo, readed - regs.rm_eo);
                *fromend = readed - regs.rm_eo;
            }
            found = true;
            return 1;
        }
        if (regexec(&done, readbuff, 1, &enma, 0) == 0) {
            printf("eolfound\n");
            r->badreq = true;
            found = false;
            if (enma.rm_eo != readed) {
                strncpy(outbuffer, readbuff + enma.rm_eo, readed - enma.rm_eo);
                *fromend = readed - enma.rm_eo;
            }

            return 0;
        }
    }

    return -2;
}

int addheadersfrombuff(Request *r, int inbufsize, int *parsed, char *inbuffer) {
    regex_t regm;
    //  regex_t done;
    int prevend = 0;
    if(inbuffer[0] == '\r' && inbuffer[1] == '\n'){
        return 0;
     }
    regcomp(&regm, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    //    regcomp(&done, "[\r][\n][\r][\r]", REG_EXTENDED|REG_NOSUB);
    regmatch_t regs;
    char *statmatch;
    printf("%d data at parsed\n", *parsed);
    int lenmatch = 0;
    int trackwhere = 0;
    printf("the read in = %s\n", inbuffer);
    while (regexec(&regm, inbuffer + trackwhere, 1, &regs, 0) == 0) {
        lenmatch = regs.rm_eo - regs.rm_so;
               printf("On header? %d to here %dwematched\n", regs.rm_eo, regs.rm_so);
        //   startofmatch = regs.rm_eo - regs.rm_so;

        statmatch = strndup(inbuffer + regs.rm_so + trackwhere, lenmatch);
        add_header(r, statmatch);
        if ((*(inbuffer + trackwhere + regs.rm_eo)) == '\r' && (*(inbuffer + trackwhere + regs.rm_eo + 1)) == '\n') {
            *parsed += regs.rm_eo + trackwhere + 2;
            printf("we found the end\n");
            return 0;
        }
        if (regs.rm_so != 0) {
            printf("the string %s flagging bad %d vs %d\n", statmatch, regs.rm_so, prevend);
            request_badflag(r);
        }
        printf("the in buffer + the regs is %s the end\n", inbuffer + trackwhere );
        prevend = regs.rm_eo;
        if (regs.rm_eo + trackwhere  == inbufsize) {
            printf("we hit the end badly parsed\n");

            *parsed += regs.rm_eo;
printf("badend parsed = %d\n", *parsed);
            return 1;
        }
        


 trackwhere +=regs.rm_eo;

    }
    printf("no matches?\n");
    *parsed += trackwhere;
    return 1;
}

int headreadstart(
    Request *r, int connfd, int inbufsize, int *fromend, char *inbuffer, char *outbuffer) {
    int toread = 2048;
    regex_t regm;
 int parser = 0;

    //    regex_t done;
    regcomp(&regm, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    //    regcomp(&done, "[\r][\n][\r][\r]", REG_EXTENDED|REG_NOSUB);
    //  regmatch_t regs;
    char readbuff[2080] = {'\0'};
    int readed = 0;
    printf("the length %lu %d\n", strlen(inbuffer), inbufsize);
    strncpy(readbuff, inbuffer, inbufsize);
    readed += inbufsize;
    int readcur = 0;
    bool found = false;
    printf("%s the starter buffer is\n", readbuff);
    //    int endofmatch = 0;
    //   char* statmatch;
    //    int lenmatch = 0;
    //  int startofmatch = 0;
       while (readed < toread || !found) {
        readcur = read(connfd, readbuff + readed, toread - readed);
        printf("toread - readed = %d\n", toread - readed);
//  parser = 0;

        printf("the amount of characters read = %d\n", readcur);
        if (readcur == 0) {
            regfree(&regm);
            printf("path 11\n");
            return -1;
        }
   //     readbuff[readed] = '\0';
              printf("the thing i read in%s\n", readbuff);
        printf("thiswhile1 parsed is %d, %d\n", parser, readcur);
        readed += readcur;
        printf("readbuff + parsed is %s\n", readbuff);
        int check = addheadersfrombuff(r, readed - parser, &parser, readbuff + parser);
        if (check == 0) {
            *fromend = readed - parser;
            printf("the total read is %d the parsed = %dreading correctly\n", readed, parser);
            strncpy(outbuffer, readbuff + parser, readed - parser);
            return 1;
        }
    }
    print_req(r);
    return -2;
}

/*int godreadstart(Request *r, char* buffer, char* returned, int connfd, bool bufin, char* regtocomp, int start, int * end, int toread){
    regex_t regm;
    regetx_t done;
    regcomp(&regm, regtocomp, REG_EXTENDED);
    regcomp(&done, "[\r][\n][\r][\n]", REG_EXTENDED);
    regmatch_t regs;
    bool reqdone = false;
    char readbuff[2048];
    strncpy(readbuff, buffer, 2048);
    int readed = 0;
    int readcur = 0;
    bool found = false;
    int endofmatch = 0;
    int startofmatch = 0;
    while(readed < toread){
      readcur = read(connfd, readbuff + start, toread - readed);
      if(readcur == 0){
        regfree(&regm);
        return -1;
      }
      readed += readcur;
      if(regexec(&regm, readbuff, 1, &regs, REG_NOTEOL)){
         lenmatch = regs.rm_eo - regs.rm_so;
         startofmatch = regs.rm_eo - regs.rm_so;
         found = true;
         break;
      
      }
      if(regexec(&done, readbuff, 1, NULL, REG_NOTEOL)){
} 
      
      

    }
    if(found){
      strncpy(returned, readbuff + regs.rm_so, lenmatch);
      *end = regs.rem_eo;
      strncpy(buffer, readbuff, 2048
      return 1;
    }else{
      return 0;
    }
    

}
*/
void add_header(Request *r, char *header_total) {
        printf("addin the header from%s\n", header_total);
    int len = strlen(header_total);
    //    printf("%s, thee length is %d\n", header_total, len);

    char *header_keyin = (char *) calloc(len + 1, sizeof(char));
    char *header_valin = (char *) calloc(len + 1, sizeof(char));
    sscanf(header_total, "%[^':']: %[^'\r']", header_keyin, header_valin);
    r->header_key[r->numheads] = header_keyin;
    r->header_vals[r->numheads] = header_valin;
    if (strcmp(header_keyin, "Content-Length") == 0) {
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
    if (r->badreq) {
        return 4;
    }
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

/*int read_somechar(int connfd, int numtoread, char* check){
   int readed = 0;
   char* buffer = (char*)calloc(numtoread, sizeof(char));
   while(readed < numtoread){
     readed += read(connfd, buffer + readed, numtoread - readed);
   }
   printf("%s\n", buffer);
   if(0 == strcmp(buffer,check)){
     printf("checked to 1");
     free(buffer);
     return 1;
    }else{
      free(buffer);
      return 0;

    }
  

}*/

/*int add_headderbuff(

    Request *r, char *buff, int start, int end, int *proced) {
    printf("add headder from buffer the start is %d  the end %d is\n", start, end);
    regex_t reghead;
    if (start == end) {
        printf("need read more cut it at the topppp\n");
        return -1;
    }
    int nummatch = 0;
    char *matchstr;
    int lenmatch = 0;
      int total_read = 0;
    //    int prevmatchend = start;
    ssize_t spot = 0;
    bool moreheads = false;
    regcomp(&reghead, "[!-~]+[:][ ]+[!-~]+", REG_EXTENDED);
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


   

    }
    


}*/
/*int add_headderbuff(

    Request *r, char *buff, int start, int end, int *proced) {
    printf("add headder from buffer the start is %d  the end %d is\n", start, end);
    regex_t reghead;
    if (start == end) {
        printf("need read more cut it at the topppp\n");
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
            printf("YOU NEED TO READ MORE HEADERS\n");
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
*/

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
                response_delete(&errrep);
                return 1;
            }
            if (errno == ENOENT) {
                Response *errrep = response_create(404);
                writeresp(errrep, connfd);
                response_delete(&errrep);

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
    }
    return 0;
}
int execute_append(Request *r, int connfd, char *buffer, int start, int end) {
    printf("PUT request\n");
    bool created = true;
    int opened = open(r->uri, O_WRONLY | O_APPEND);
    if (opened == -1) {
        printf("not openend right\n");
        if (errno == ENOENT) {
            printf("bad access somehow\n");
            Response *errrep = response_create(404);
            writeresp(errrep, connfd);
            response_delete(&errrep);
            return 1;
        }
        if (errno == EACCES) {
            printf("bad access somehow\n");
            Response *errrep = response_create(403);
            writeresp(errrep, connfd);
            response_delete(&errrep);
            return 1;
        }
    }
    bool readfrombuf = false;
    if (start != end) {
        readfrombuf = true;
    }

    int remain = end - start;
    printf("the buff %d the write%d\n", start - end, r->content_len);
    if (remain >= r->content_len) {
        printf("written loop 1\n");
        write(opened, buffer + start, r->content_len);

    } else {
        if (end - start != 0) {
            write(opened, buffer + start, end - start);
        }
        int writed = end - start;

        char bufftwo[2048];
        int readed;
        while (writed < r->content_len) {
            //  printf("in the write loop written %d, need to writed %d\n");
            readed = read(connfd, bufftwo, 2048);
            write(opened, bufftwo, readed);
            printf("%s", bufftwo);
            writed += readed;
        }
    }
    close(opened);
    if (!created) {
        Response *resp = response_create(201);
        writeresp(resp, connfd);
        response_delete(&resp);
    } else {
        Response *resp = response_create(200);
        writeresp(resp, connfd);
        response_delete(&resp);
    }

    return 0;
}

int execute_put(Request *r, int connfd, char *buffer, int start, int end) {
    printf("PUT request\n");
    bool created = true;
    int opened = open(r->uri, O_CREAT | O_EXCL | O_WRONLY, 0666);
    if (opened == -1) {
        printf("not openend right\n");
        if (errno == EEXIST) {
            printf("the swag thing\n");
            created = false;
            opened = open(r->uri, O_WRONLY);
            if (opened == -1) {
                if (errno == EACCES) {
                    Response *errrep = response_create(403);
                    writeresp(errrep, connfd);
                    response_delete(&errrep);
                    return 1;
                }
            }
        }
        if (errno == EACCES) {
            printf("bad access somehow\n");
            Response *errrep = response_create(403);
            writeresp(errrep, connfd);
            response_delete(&errrep);
            return 1;
        }
    }
    bool readfrombuf = false;
    if (start != end) {
        readfrombuf = true;
    }

    int remain = end - start;
    printf("the buff %d the write%d\n", start - end, r->content_len);
    if (remain >= r->content_len) {
        printf("written loop 1\n");
        write(opened, buffer + start, r->content_len);

    } else {
        if (end - start != 0) {
            write(opened, buffer + start, end - start);
        }
        int writed = end - start;

        char bufftwo[2048];
        int readed = 0;
        while (writed < r->content_len) {
            //  printf("in the write loop written %d, need to writed %d\n");
            readed = read(connfd, bufftwo, 2048);
            write(opened, bufftwo, readed);
            // printf("%s", bufftwo);
            writed += readed;
        }
    }

    Response *resp = response_create(200);
    writeresp(resp, connfd);
    response_delete(&resp);

    return 0;
}
