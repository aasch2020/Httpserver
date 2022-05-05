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
    int resptype;
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
    r->resptype = 0;
    return r;
}
void request_badflag(Request *r) {
    r->badreq = true;
}
void request_update(Request *r, char *match) {
    r->uri[0] = '.';
    //  r->thetotalhead = strndup(match, 40);
    //   printf("update req;");
    sscanf(match, "%8[a-zA-Z] %22s HTTP/%u.%u", r->type, r->uri + 1, &(r->vernum), &(r->verdec));

    print_req(r);
}

void request_clear(Request *r) {
    for (int i = 0; i < r->numheads; i++) {
        free(r->header_key[i]);
        free(r->header_vals[i]);
    }
    memset(r->uri, '\0', 22);
    memset(r->uri, '\0', 9);
    r->numheads = 0;
    r->badreq = false;
    r->vernum = -1;
    r->verdec = -1;
    r->resptype = 0;
    r->content_len = -1;
}
void request_delete(Request **r) {
    if (*r != NULL) {
        for (int i = 0; i < ((*r)->numheads); i++) {
            free((*r)->header_key[i]);
            free((*r)->header_vals[i]);
        }
        free((*r)->header_vals);
        free((*r)->header_key);
        free(*r);
        *r = NULL;
    }
}
int hcreadstart(
    Request *r, int connfd, int inbufsize, int *fromend, char *inbuffer, char *outbuffer) {
    int toread = 39;
    printf("the buffer in is length %d at hcreadstart %s\n", inbufsize, inbuffer);
    regex_t regm;
    regex_t done;
    regmatch_t regs;
    regmatch_t enma;
    char readbuff[1024] = { '\0' };
    strncpy(readbuff, inbuffer, 40);
    int readed = inbufsize;
    int readcur = 0;
    bool found = false;
    //    int endofmatch = 0;
    char *statmatch;
    int lenmatch = 0;
    //    int startofmatch = 0;
    //  if(inbufsize > toread){

    //  }
    int timesgone = 0;
    while (readed < toread) {

        if (!(inbufsize > 0 && timesgone == 0)) {
            readcur = read(connfd, readbuff + readed, toread - readed);
            printf("readbuff %s\n, readbuff", readbuff);
            if (readcur == 0) {
                //   regfree(&regm);
                //  regfree(&done);
                printf("path 1\n");
                return -1;
            }
        }
        regcomp(&regm,
            "[a-z, A-Z]{1,8}[ ][/][a-zA-Z0-9._/]{1,19}[ ][H][T][T][P][/][0-9][.][0-9][\r][\n]",
            REG_EXTENDED);
        regcomp(&done, "[\r][\n]", REG_EXTENDED);

        timesgone++;
        //    printf("thiswhile read is %d\n", readcur);
        readed += readcur;
        //   printf("readbuff %s", readbuff);
        if (0 == regexec(&regm, readbuff, 1, &regs, 0)) {
            //         printf("wematchedstat\n");
            lenmatch = regs.rm_eo - regs.rm_so;
            //       printf("%d to here %dwematched\n", regs.rm_so, regs.rm_eo);
            //   startofmatch = regs.rm_eo - regs.rm_so;
            statmatch = strndup(readbuff + regs.rm_so, lenmatch);
            //      printf("the string we matched is%s\n", statmatch);
            request_update(r, statmatch);
            if (regs.rm_eo != readed) {
                strncpy(outbuffer, readbuff + regs.rm_eo, readed - regs.rm_eo);
                *fromend = readed - regs.rm_eo;
            }
            regfree(&done);
            regfree(&regm);
            free(statmatch);
            found = true;
            return 1;
        }
        if (0 == regexec(&done, readbuff, 1, &enma, 0)) {
            printf("eolfound\n");
            r->badreq = true;
            found = false;
            if (enma.rm_so != readed) {
                strncpy(outbuffer, readbuff + enma.rm_eo, readed - enma.rm_eo);
                printf("when we find eol %d %d\n", enma.rm_so, readed);
                *fromend = readed - enma.rm_eo;
            }
            regfree(&done);
            regfree(&regm);
            return 0;
        }
    }
    printf("non match)\n");
    //   regfree(&done);
    //    regfree(&regm);
    r->badreq = true;
    return 0;
}

int addheadersfrombuff(Request *r, int inbufsize, int *parsed, char *inbuffer) {
    regex_t regm;
    regex_t done;
    int prevend = 0;
    if (inbuffer[0] == '\r' && inbuffer[1] == '\n') {
        printf("matching early proc parsed = %d  we may be done here\n", *parsed);
        *parsed += 2;
        return 0;
    }
    regcomp(&regm, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    regcomp(&done, "[\r][\n][\r][\r]", REG_EXTENDED | REG_NOSUB);
    regmatch_t regs;
    char *statmatch;
    printf("%d data at parsed\n", *parsed);
    int lenmatch = 0;
    int trackwhere = 0;
    printf("the read in = %s\n", inbuffer);
    while ((regexec(&regm, inbuffer + trackwhere, 1, &regs, 0) == 0) && (trackwhere != inbufsize)) {
        lenmatch = regs.rm_eo - regs.rm_so;
        printf("On header? %d to here %dwematched\n", regs.rm_eo, regs.rm_so);
        //   startofmatch = regs.rm_eo - regs.rm_so;

        statmatch = strndup(inbuffer + regs.rm_so + trackwhere, lenmatch);
        add_header(r, statmatch);
        if ((*(inbuffer + trackwhere + regs.rm_eo)) == '\r'
            && (*(inbuffer + trackwhere + regs.rm_eo + 1)) == '\n') {
            *parsed += regs.rm_eo + trackwhere + 2;
            printf("we found the end\n");
            regfree(&done);
            regfree(&regm);
            free(statmatch);
            return 0;
        }
        if (regs.rm_so != 0) {
            printf("the string %s flagging bad %d vs %d\n", statmatch, regs.rm_so, prevend);
            request_badflag(r);
        }
        printf("the in buffer + the regs is %s the end\n", inbuffer + trackwhere);
        prevend = regs.rm_eo;
        free(statmatch);
        trackwhere += regs.rm_eo;
    }
    //  printf("before end cond we have");
    if (regexec(&done, inbuffer + trackwhere, 0, NULL, 0) == 0) {
        printf("verybadend\n");
        r->badreq = true;
        regfree(&done);
        regfree(&regm);
        return 0;
    }
    printf("out the end now");
    // printf("no matches?\n");
    *parsed += trackwhere;
    regfree(&done);
    regfree(&regm);
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
    char readbuff[2080] = { '\0' };
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
    int timesgone = 0;
    while (readed < toread || !found) {
        if (!(inbufsize > 0 && timesgone == 0)) {
            printf("stuck reading\n");
            readcur = read(connfd, readbuff + readed, toread - readed);
            printf("toread - readed = %d\n", toread - readed);
            //  parser = 0;

            printf("the amount of characters read = %d\n", readcur);
            if (readcur == 0) {
                regfree(&regm);
                printf("path 11\n");
                return -1;
            }
        }
        timesgone++;
        //     readbuff[readed] = '\0';
        printf("the thing i read in%s\n", readbuff);
        printf("thiswhile1 parsed is %d, %d\n", parser, readcur + parser);
        readed += readcur;
        printf("readbuff + parsed is %s\n", readbuff);
        int check = addheadersfrombuff(r, readed - parser, &parser, readbuff + parser);
        if (check == 0) {
            *fromend = readed - parser;
            regfree(&regm);
            printf("the total read is %d the parsed = %dreading correctly\n", readed, parser);
            strncpy(outbuffer, readbuff + parser, readed - parser);
            return 1;
        }
    }
    //   print_req(r);
    printf("what is this end condition?\n");
    regfree(&regm);
    return 0;
}

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
    if (r->vernum != 1 || r->verdec != 1) {
        return 4;
    }
    if (r->badreq) {
        return 4;
    }

    if (strcmp(r->type, "GET") == 0) {
        return 1;
    }
    if (strcmp(r->type, "PUT") == 0) {
        if (r->content_len == -1) {
            printf("the cont len == %d\n", r->content_len);
            return 4;
        }
        return 2;
    }
    if (strcmp(r->type, "APPEND") == 0) {
        if (r->content_len == -1) {
            return 4;
        }

        return 3;
    }
    return 0;
}

const char *get_uri(Request *r) {
    return r->uri;
}

int execute_get(Request *r, int connfd) {
    int types = type(r);
    if (types == 1) {
        printf("doing a get req");

        int opened = open(r->uri, O_RDWR);
        if (errno == EISDIR) {
            Response *errrep = response_create(403);
            writeresp(errrep, connfd);
            response_delete(&errrep);
            return 1;
        }
        //     int error = errno;
        opened = open(r->uri, O_RDONLY);
        if (opened == -1) {
            if ((errno == EACCES) || (errno == EISDIR)) {
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
int execute_append(
    Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf, int inbufsize) {
    printf("PUT request\n");
    //   bool created = true;
    int resptype = 0;
    int opened = open(r->uri, O_WRONLY | O_APPEND);
    if (opened == -1) {
     int errord = errno;
     printf("not openend right\n");
        if (errord == ENOENT) {
            resptype = 404;
            //       return 1;
        }
        if ((errord == EACCES) ||(errord == EISDIR)) {
            printf("bad access somehow\n");
            resptype = 403;
            //     return 1;
        }
        printf("the error number is %d\n", errord);

    } else {
         printf("strting to try prin\n");
        int writed = 0;
        //    int towrite = r->content_len;
        if (inbufsize >= r->content_len) {
            printf("writing here too but shouldn");
            writed = write(opened, buffer, r->content_len);
            strncpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);
            *fromend = inbufsize - r->content_len;

        } else {
            int totalwrote = 0;
            int remain = 0;
            if (inbufsize != 0) {
                remain = r->content_len - inbufsize;
                write(opened, buffer, remain);
            }
            int readed = 0;
            totalwrote += remain;
            char bufftwo[1024] = { '\0' };
            printf("%d, remain, %d, totalwrote", remain, totalwrote);
            while (totalwrote < r->content_len) {
                printf("stuck here\n");
                printf("%d, remain, %d, totalwrote", remain, totalwrote);
                //  printf("in the write loop written %d, need to writed %d\n");
                if (r->content_len - totalwrote > 1024) {
                    readed = read(connfd, bufftwo, 1024);
                    totalwrote += 1024;
                } else {
                    readed = read(connfd, bufftwo, r->content_len - totalwrote);
                    totalwrote += readed;
                }
                if(readed == 0){
                    return -1;
                }
                write(opened, bufftwo, readed);
                //   printf("%s", bufftwo);
            }
   
       }
    resptype = 200;
        close(opened);
    }
    Response *resp = response_create(resptype);
    writeresp(resp, connfd);
    response_delete(&resp);
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
int execute_put(
    Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf, int inbufsize) {
    printf("PUT request\n");
    bool created = true;
    int resptype = 0;
    bool opens = false;
    int opened = open(r->uri, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (opened == -1) {
        printf("not openend right\n");
        if (errno == EEXIST) {
            printf("the swag thing\n");
            created = false;
            opens = true;
            opened = open(r->uri, O_WRONLY);
            if ((errno == EACCES) || (errno == EISDIR)) {
                printf("bad access somehow\n");
                resptype = 403;
                opens = false;
            }
        }
        if ((errno == EACCES) || (errno == EISDIR)) {
            printf("death");
            resptype = 403;
            opens = false;
        }
    } else {
        opens = true;
    }
    if (opens) {
        printf("strting to try prin\n");
        int writed = 0;
        //    int towrite = r->content_len;
        if (inbufsize >= r->content_len) {
            printf("writing here too but shouldn");
            writed = write(opened, buffer, r->content_len);
            strncpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);
            *fromend = inbufsize - r->content_len;

        } else {
            int totalwrote = 0;
            int remain = 0;
            if (inbufsize != 0) {
                remain = r->content_len - inbufsize;
                write(opened, buffer, remain);
            }
            int readed = 0;
            totalwrote += remain;
            char bufftwo[1024] = { '\0' };
            printf("%d, remain, %d, totalwrote", remain, totalwrote);
            while (totalwrote < r->content_len) {
                printf("stuck here\n");
                printf("%d, remain, %d, totalwrote", remain, totalwrote);
                //  printf("in the write loop written %d, need to writed %d\n");
                if (r->content_len - totalwrote > 1024) {
                    readed = read(connfd, bufftwo, 1024);
                    totalwrote += 1024;
                } else {
                    readed = read(connfd, bufftwo, r->content_len - totalwrote);
                    totalwrote += readed;
                }
if(readed == 0){
                    return -1;
                }

                write(opened, bufftwo, readed);
                //   printf("%s", bufftwo);
            }
        }
        resptype = 200;
        close(opened);
    }

    if (created) {

        Response *resp = response_create(201);
        writeresp(resp, connfd);
        response_delete(&resp);

    } else {

        Response *resp = response_create(resptype);
        writeresp(resp, connfd);
        response_delete(&resp);
    }
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
