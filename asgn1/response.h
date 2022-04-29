typedef struct Response Response;

Response *response_create(int type);

void response_delete(Response **r);

void addheaderres(Response *r, char *header_keyin, char *header_valin);

void response_delete(Response **r);

void write_nofile(Response *r, int connec);

void write_file(Response *r, int filewrt, int connec);
