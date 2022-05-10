#include "request.h"
#include <fcntl.h>

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
    int Reqid;
    //    char* thetotalhead;
};

void writelog(Request *r, Response *a, FILE* logfile){
    fprintf(logfile, "%s,%s,%d,%d\n", get_type(r), get_uri(r), resptype(a), reqid(r));
    fflush(logfile);
 
  }

Request *request_create() {
    Request *r = (Request *) calloc(1, sizeof(Request));
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));
    r->numheads = 0;
    r->content_len = -1;
    r->numheads = 0;
    r->badreq = false;
    r->resptype = 0;
    r->Reqid = 0;
    return r;
}
void request_badflag(Request *r) {
    r->badreq = true;
}

int reqid(Request *r){
  return r->Reqid;

}
void request_update(Request *r, char *match) {
    sscanf(match, "%8[a-zA-Z] %22s HTTP/%u.%u", r->type, r->uri, &(r->vernum), &(r->verdec));
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
    r->Reqid = 0;
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
    regex_t regm;
    regex_t done;
    regmatch_t regs;
    regmatch_t enma;
    char readbuff[1024] = { '\0' };
    strncpy(readbuff, inbuffer, 40);
    int readed = inbufsize;
    int readcur = 0;
    bool found = false;
    char *statmatch;
    int lenmatch = 0;
    int timesgone = 0;
    while (readed < toread) {

        if (!(inbufsize > 0 && timesgone == 0)) {
            readcur = read(connfd, readbuff + readed, toread - readed);
            if (readcur == 0) {
                return -1;
            }
        }
        regcomp(&regm,
            "[a-z, A-Z]{1,8}[ ][/][a-zA-Z0-9._/]{1,19}[ ][H][T][T][P][/][0-9][.][0-9][\r][\n]",
            REG_EXTENDED);
        regcomp(&done, "[\r][\n]", REG_EXTENDED);

        timesgone++;
        readed += readcur;
        if (0 == regexec(&regm, readbuff, 1, &regs, 0)) {
            lenmatch = regs.rm_eo - regs.rm_so;
            statmatch = strndup(readbuff + regs.rm_so, lenmatch);
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
            r->badreq = true;
            found = false;
            if (enma.rm_so != readed) {
                strncpy(outbuffer, readbuff + enma.rm_eo, readed - enma.rm_eo);
                *fromend = readed - enma.rm_eo;
            }
            regfree(&done);
            regfree(&regm);
            return 0;
        }
    }
    r->badreq = true;
    return 0;
}

int addheadersfrombuff(Request *r, int inbufsize, int *parsed, char *inbuffer) {
    regex_t regm;
    regex_t done;
    int prevend = 0;
    if (inbuffer[0] == '\r' && inbuffer[1] == '\n') {
        *parsed += 2;
        return 0;
    }
    regcomp(&regm, "[!-~]+[:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    regcomp(&done, "[\r][\n][\r][\n]", REG_EXTENDED | REG_NOSUB);
    regmatch_t regs;
    char *statmatch;
    int lenmatch = 0;
    int trackwhere = 0;
    while ((regexec(&regm, inbuffer + trackwhere, 1, &regs, 0) == 0) && (trackwhere != inbufsize)) {
        lenmatch = regs.rm_eo - regs.rm_so;
        statmatch = strndup(inbuffer + regs.rm_so + trackwhere, lenmatch);
        add_header(r, statmatch);
        if ((*(inbuffer + trackwhere + regs.rm_eo)) == '\r'
            && (*(inbuffer + trackwhere + regs.rm_eo + 1)) == '\n') {
            *parsed += regs.rm_eo + trackwhere + 2;
            regfree(&done);
            regfree(&regm);
            free(statmatch);
            return 0;
        }
        if (regs.rm_so != 0) {
            request_badflag(r);
        }
        prevend = regs.rm_eo;
        free(statmatch);
        trackwhere += regs.rm_eo;
    }
    if (regexec(&done, inbuffer + trackwhere, 0, NULL, 0) == 0) {
        r->badreq = true;
        regfree(&done);
        regfree(&regm);
        return 0;
    }
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

    regcomp(&regm, "[!-~]+[0-9a-zA-Z][:][ ]+[!-~]+[\r][\n]", REG_EXTENDED);
    char readbuff[2080] = { '\0' };
    int readed = 0;
    strncpy(readbuff, inbuffer, inbufsize);
    readed += inbufsize;
    int readcur = 0;
    bool found = false;
    int timesgone = 0;
    while (readed < toread || !found) {
        if (!(inbufsize > 0 && timesgone == 0)) {
            readcur = read(connfd, readbuff + readed, toread - readed);
            if (readcur == 0) {
                regfree(&regm);
                return -1;
            }
        }
        timesgone++;
        readed += readcur;
        int check = addheadersfrombuff(r, readed - parser, &parser, readbuff + parser);
        if (check == 0) {
            *fromend = readed - parser;
            regfree(&regm);
            strncpy(outbuffer, readbuff + parser, readed - parser);
            return 1;
        }
    }
    regfree(&regm);
    return 0;
}

void add_header(Request *r, char *header_total) {
    int len = strlen(header_total);
    char *header_keyin = (char *) calloc(len + 1, sizeof(char));
    char *header_valin = (char *) calloc(len + 1, sizeof(char));
    sscanf(header_total, "%[^':']: %[^'\r']", header_keyin, header_valin);
    r->header_key[r->numheads] = header_keyin;
    r->header_vals[r->numheads] = header_valin;
    if (strcmp(header_keyin, "Content-Length") == 0) {
        r->content_len = atoi(header_valin);
    }
    if (strcmp(header_keyin, "Request-Id") == 0) {
        r->Reqid = atoi(header_valin);
    }
   
    r->numheads += 1;
    if (r->numheads % 30 == 0) {
        r->header_key = realloc(r->header_key, (r->numheads + 30) * sizeof(char *));

        r->header_vals = realloc(r->header_vals, (r->numheads + 30) * sizeof(char *));
    }
}

void print_req(Request *r) {
    for (int i = 0; i < r->numheads; i++) {
        printf("%s\n", r->header_key[i]);
        printf("%s\n", r->header_vals[i]);
    }
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
const char *get_type(Request *r) {
    return r->type;
}


int execute_get(Request *r, int connfd, FILE* logfile) {
    int types = type(r);
    if (types == 1) {
        int opened = open(r->uri + 1, O_RDWR);
        if (errno == EISDIR) {
            Response *errrep = response_create(403);
writelog(r, errrep,logfile);
            writeresp(errrep, connfd);
            response_delete(&errrep);
            return 1;
        }
        //     int error = errno;
        opened = open(r->uri + 1, O_RDONLY);
        if (opened == -1) {
            if ((errno == EACCES) || (errno == EISDIR)) {
                Response *errrep = response_create(403);
                writeresp(errrep, connfd);
writelog(r, errrep,logfile);

                response_delete(&errrep);
                return 1;
            } else if (errno == ENOENT) {
                Response *errrep = response_create(404);
                writeresp(errrep, connfd);
writelog(r, errrep,logfile);

                response_delete(&errrep);

                return 1;
            } else {
                Response *errrep = response_create(500);
                writeresp(errrep, connfd);
writelog(r, errrep,logfile);

                response_delete(&errrep);
                return 1;
            }
        }
        int resptype = 200;
        Response *resp = response_create(resptype);
        write_file(resp, opened, connfd);
writelog(r, resp,logfile);

        response_delete(&resp);
    }
    return 0;
}
int execute_append(
    Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf, int inbufsize, FILE* logfile) {
    int resptype = 0;
    int opened = open(r->uri + 1, O_WRONLY | O_APPEND);
    if (opened == -1) {
        int errord = errno;
        if (errord == ENOENT) {
            resptype = 404;
        } else if ((errord == EACCES) || (errord == EISDIR)) {
            resptype = 403;
        } else {

            resptype = 500;
        }
    } else {
        int writed = 0;
        if (inbufsize >= r->content_len) {
            writed = write(opened, buffer, r->content_len);
            memcpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);
            *fromend = inbufsize - r->content_len;

        } else {
            int totalwrote = 0;
            if (inbufsize != 0) {
                write(opened, buffer, inbufsize);
            }
            int readed = 0;
            totalwrote += inbufsize;
            char bufftwo[1025] = { '\0' };
            while (totalwrote < r->content_len) {
                if (r->content_len - totalwrote >= 1024) {
                    readed = read(connfd, bufftwo, 1024);
                    totalwrote += readed;
                } else {
                    readed = read(connfd, bufftwo, r->content_len - totalwrote);
                    totalwrote += readed;
                }
                if (readed == 0) {
                    close(opened);
                    resptype = 501;
                    return 1;
                }

                write(opened, bufftwo, readed);
            }
        }
        resptype = 200;
        close(opened);
    }
    Response *resp = response_create(resptype);
    writeresp(resp, connfd);
writelog(r, resp,logfile);


    response_delete(&resp);
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
int execute_put(
    Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf, int inbufsize, FILE* logfile) {
    bool created = false;
    int resptype = 0;
    bool opens = false;
    int opened = open(r->uri + 1, O_WRONLY);
    if (opened == -1) {
        if (errno == ENOENT) {
            created = false;
            opens = true;
            opened = open(r->uri + 1, O_WRONLY | O_CREAT);
            if ((errno == EACCES) || (errno == EISDIR)) {
                resptype = 403;
                opens = false;
            } else {
                created = true;
            }
        } else if ((errno == EACCES) || (errno == EISDIR)) {
            resptype = 403;
            opens = false;
        } else {
            opens = false;
            resptype = 500;
        }
    } else {
        opens = true;
    }
    if (opens) {
        int writed = 0;
        if (inbufsize >= r->content_len) {
            writed = write(opened, buffer, r->content_len);
            memcpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);
            *fromend = inbufsize - r->content_len;

        } else {
            int totalwrote = 0;
            if (inbufsize != 0) {
                write(opened, buffer, inbufsize);
            }
            int readed = 0;
            totalwrote += inbufsize;
            char bufftwo[1025] = { '\0' };
            while (totalwrote < r->content_len) {
                if (r->content_len - totalwrote >= 1024) {
                    readed = read(connfd, bufftwo, 1024);
                    totalwrote += readed;
                } else {
                    readed = read(connfd, bufftwo, r->content_len - totalwrote);
                    totalwrote += readed;
                }
                if (readed == 0) {
                    return -1;
                }

                write(opened, bufftwo, readed);
            }
        }
        resptype = 200;
        close(opened);
    }

    if (created) {

        Response *resp = response_create(201);
        writeresp(resp, connfd);
writelog(r, resp,logfile);


        response_delete(&resp);

    } else {

        Response *resp = response_create(resptype);
        writeresp(resp, connfd);
writelog(r, resp,logfile);


        response_delete(&resp);
    }
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
