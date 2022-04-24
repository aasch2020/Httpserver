typedef struct Request Request;

Request *request_create(char *type, char *uri, int vernum, int verdec);

void request_delete(Request **r);

void add_header(Request *r, char *header_key, char *header_val);

int add_headderbuff(Request *r, char* buff, int start, char *header_key, char *header_val);

void print_req(Request *r);

int validate(Request *r);

int type(Request *r);

const char* get_uri(Request *r);
