struct Response{
  char statphrase[25];
  char **header_key;
  char **header_vals;
  char *msgbod;
}

Response *response_create(int type){
  switch(type)

}

void response_delete(Response **r);
