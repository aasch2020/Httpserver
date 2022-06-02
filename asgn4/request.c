#include "request.h"
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>
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
void writelog(Request *r, Response *a, FILE *logfile) {
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

int reqid(Request *r) {
    return r->Reqid;
}
void request_update(Request *r, char *match) {
    sscanf(match, "%8[a-zA-Z] %22s HTTP/%u.%u", r->type, r->uri, &(r->vernum), &(r->verdec));
}
void request_init(Request *r) {
    r->header_key = (char **) calloc(30, sizeof(char *));
    r->header_vals = (char **) calloc(30, sizeof(char *));

    r->numheads = 0;
    r->content_len = -1;
    r->numheads = 0;
    r->badreq = false;
    r->resptype = 0;
    r->Reqid = 0;
}
void request_clear(Request *r) {

    r->content_len = -1;
    r->badreq = false;
    r->resptype = 0;
    r->Reqid = 0;

    for (int i = 0; i <= r->numheads; i++) {
        free(r->header_key[i]);
        free(r->header_vals[i]);
        r->header_key[i] = NULL;
        r->header_vals[i] = NULL;
    }
    r->numheads = 0;
    free(r->header_key);
    free(r->header_vals);
}
void request_delete(Request **r) {
    if (*r != NULL) {
        for (int i = 0; i <= ((*r)->numheads); i++) {
            free((*r)->header_key[i]);
            free((*r)->header_vals[i]);
        }
        free(*r);
        *r = NULL;
    }
    fflush(stdin);
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
    memcpy(readbuff, inbuffer, inbufsize);
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
            memcpy(outbuffer, readbuff + parser, readed - parser);
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
    r->numheads++;

    sscanf(header_total, "%[^':']: %[^'\r']", header_keyin, header_valin);
    r->header_key[r->numheads] = header_keyin;
    r->header_vals[r->numheads] = header_valin;
    if (strcmp(header_keyin, "Content-Length") == 0) {
        r->content_len = atoi(header_valin);
    }
    if (strcmp(header_keyin, "Request-Id") == 0) {
        r->Reqid = atoi(header_valin);
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

pthread_mutex_t filechecklock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t loglock = PTHREAD_MUTEX_INITIALIZER;
int execute_get(Request *r, int connfd, FILE *logfile) {
    pthread_mutex_lock(&filechecklock);
    int opened = open(r->uri + 1, O_RDONLY);
    if (opened == -1) {
        if ((errno == EACCES) || (errno == EISDIR)) {
            Response *errrep = response_create(403);

            writeresp(errrep, connfd);
            writelog(r, errrep, logfile);
            pthread_mutex_unlock(&filechecklock);

            response_delete(&errrep);
            return 1;
        } else if (errno == ENOENT) {
            Response *errrep = response_create(404);
            writeresp(errrep, connfd);
            writelog(r, errrep, logfile);
            pthread_mutex_unlock(&filechecklock);

            response_delete(&errrep);

            return 1;
        } else {
            Response *errrep = response_create(500);
            writeresp(errrep, connfd);
            writelog(r, errrep, logfile);
            pthread_mutex_unlock(&filechecklock);

            response_delete(&errrep);
            return 1;
        }
    } else {
        flock(opened, LOCK_SH);
        pthread_mutex_unlock(&filechecklock);
    }
    int resptype = 200;
    Response *resp = response_create(resptype);

    write_file(resp, opened, connfd);

    writelog(r, resp, logfile);

    flock(opened, LOCK_UN);
    response_delete(&resp);

    return 0;
}
int execute_append(Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf,
    int inbufsize, FILE *logfile) {
    int resptype = 0;
    char templ[8] = "tXXXXXX";
    int tempfd = mkstemp(templ);

    int writed = 0;
    if (inbufsize >= r->content_len) {
        writed = write(tempfd, buffer, r->content_len);
        memcpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);

        *fromend = inbufsize - r->content_len;
    } else {
        int totalwrote = 0;
        if (inbufsize != 0) {

            write(tempfd, buffer, inbufsize);
        }
        ssize_t readed = 0;
        totalwrote += inbufsize;
        char bufftwo[2048] = { '\0' };
        while (totalwrote < r->content_len) {
            if (r->content_len - totalwrote >= 1024) {
                readed = read(connfd, bufftwo, 1024);
                totalwrote += readed;
            } else {

                readed = read(connfd, bufftwo, r->content_len - totalwrote);
                totalwrote += readed;
            }
            write(tempfd, bufftwo, readed);
            if (readed == 0) {

                resptype = 501;
                return 1;
            }
            readed = 0;
        }
    }
    resptype = 200;
    pthread_mutex_lock(&filechecklock);
    int opened = open(r->uri + 1, __O_PATH);
    if (opened == -1) {
        int errord = errno;
        if (errord == ENOENT) {
            resptype = 404;
            Response *resp = response_create(resptype);
            writeresp(resp, connfd);
            writelog(r, resp, logfile);

            pthread_mutex_unlock(&filechecklock);
            response_delete(&resp);
            return 1;
        }
    } else {

        flock(opened, LOCK_EX);
        pthread_mutex_unlock(&filechecklock);
    }
    int connopen = open(r->uri + 1, O_WRONLY | O_APPEND);
    int transfer = 0;
    int numwrite = r->content_len;
    char buffs[2048];
    int readed = 0;
    while (transfer < numwrite) {
        readed = read(tempfd, buffs, 2048);
        write(connopen, buffs, readed);
        transfer += numwrite;
    }
    flock(opened, LOCK_UN);

    close(opened);
    Response *resp = response_create(resptype);
    writeresp(resp, connfd);

    writelog(r, resp, logfile);
    response_delete(&resp);
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
int execute_put(Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf,
    int inbufsize, FILE *logfile) {
    bool created = false;
    int resptype = 0;
    bool opens = true;
    char templ[8] = "tXXXXXX";
    int tempfd = mkstemp(templ);
    if (tempfd == -1) {
        printf("error making temp file\n");
    }
    if (opens) {
        int writed = 0;
        if (inbufsize >= r->content_len) {
            writed = write(tempfd, buffer, r->content_len);
            memcpy(writtenfrombuf, buffer + writed, inbufsize - r->content_len);
            *fromend = inbufsize - r->content_len;
        } else {
            int totalwrote = 0;
            if (inbufsize != 0) {
                write(tempfd, buffer, inbufsize);
            }

            ssize_t readed = 0;
            totalwrote += inbufsize;
            char bufftwo[2048] = { '\0' };

            while (totalwrote < r->content_len) {
                if (r->content_len - totalwrote >= 1024) {
                    readed = read(connfd, bufftwo, 1024);
                    totalwrote += readed;
                } else {
                    readed = read(connfd, bufftwo, r->content_len - totalwrote);
                    totalwrote += readed;
                }
                write(tempfd, bufftwo, readed);
                if (readed == 0) {
                    printf("the connection died\n");
                    return -1;
                }
                readed = 0;
            }
        }
        resptype = 200;
    }
    pthread_mutex_lock(&filechecklock);

    int opened = open(r->uri + 1, __O_PATH);
    int createdfd = 0;
    created = false;
    if (opened == -1) {
        created = true;
        createdfd = open(r->uri + 1, O_CREAT | O_RDWR, 0666);
        flock(createdfd, LOCK_EX);
        pthread_mutex_unlock(&filechecklock);

    } else {
        flock(opened, LOCK_EX);
        pthread_mutex_unlock(&filechecklock);
    }
    if (rename(templ, r->uri + 1) != 0) {
        printf("rename more like relame\n");
    }
    remove(templ);
    if (created) {
        Response *resp = response_create(201);
        writeresp(resp, connfd);

        writelog(r, resp, logfile);
        flock(createdfd, LOCK_UN);

        response_delete(&resp);
    } else {
        Response *resp = response_create(resptype);
        writeresp(resp, connfd);

        writelog(r, resp, logfile);
        flock(opened, LOCK_UN);
        response_delete(&resp);
    }
    if (resptype != 200) {
        return 1;
    }
    return 0;
}
