#include <stdio.h>
#include <stdbool.h>
typedef struct Request Request;
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

void request_init(Request *r);
Request *request_create();
void request_update(Request *r, char *match);
int headreadstart(
    Request *r, int connfd, int inbufsize, int *fromend, char *inbuffer, char *outbuffer);
void request_delete(Request **r);
int hcreadstart(
    Request *r, int connfd, int inbufsize, int *fromend, char *inbuffer, char *outbuffer);
void add_header(Request *r, char *header_total);

void request_clear(Request *r);

int add_headderbuff(Request *r, char *buff, int start, int end, int *proced);

void print_req(Request *r);

int validate(Request *r);
int execute_append(Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf,
    int inbufsize, FILE *logfile);
int reqid(Request *r);
int type(Request *r);
int execute_put(Request *r, int connfd, char *buffer, int *fromend, char *writtenfrombuf,
    int inbufsize, FILE *logfile);
int execute_get(Request *r, int connfd, FILE *logfile);
const char *get_type(Request *r);
const char *get_uri(Request *r);
