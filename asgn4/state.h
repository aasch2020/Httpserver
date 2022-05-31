struct Request;
typedef struct State State;
struct State {
    int connec;
    int where;
    Request storeq;
    char holding[4096];
    int lenhold;
};

void setState(State *states, int connfd, int where, Request req, char *holdings, int lenhold);

void startstate(State *states, int connfd);
