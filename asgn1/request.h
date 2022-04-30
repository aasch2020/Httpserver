typedef struct Request Request;

Request *request_create();
Request *request_update( Request *r, char *type, char *uri, int vernum, int verdec);

void request_delete(Request **r);

void add_header(Request *r, char *header_total);

int add_headderbuff(Request *r, char *buff, int start, int end);

void print_req(Request *r);

int validate(Request *r);

int type(Request *r);

int execute_req(Request *r, int connfd);

const char *get_uri(Request *r);
