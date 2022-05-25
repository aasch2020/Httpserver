#include <string.h>
struct Request;
void setState(State *states, int connfd, int where, Request req, char *holdings, int lenhold) {
    states->connec = connfd;
    states->where = where;
    states->storeq = req;
    memcpy(states->holding, holdings, lenhold);
    states->lenhold = lenhold;
}

void startstate(State *states, int connfd) {
    states->connec = connfd;
    states->where = 0;
    memset(states->holding, '\0', 4096);
    states->lenhold = 0;
}
