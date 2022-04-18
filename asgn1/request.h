typedef struct Request Request;

Request *request_create(char *type, char *uri, int vernum, int verdec);

void request_delete(Request **r);

void add_header(Request *r, char *header);

void print_req(Request *r);

int validate(Request *r);
