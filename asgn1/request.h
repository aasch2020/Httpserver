typedef struct Request Request;

Request *request_create(char *match);

void request_delete(Request **r);

void add_header(Request *r, char *header_total);

int add_headderbuff(Request *r, char *buff, int start, int end, int* proced);

void print_req(Request *r);

int validate(Request *r);
 int execute_append(Request *r, int connfd, char* buffer, int start, int end);

int type(Request *r);
int execute_put(Request *r, int connfd, char* buffer, int start, int end);
int execute_get(Request *r, int connfd);

const char *get_uri(Request *r);
