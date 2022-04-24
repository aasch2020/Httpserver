typedef struct Response Response;

Response *response_create(char *type, char *uri, int vernum, int verdec);

void response_delete(Response **r);
