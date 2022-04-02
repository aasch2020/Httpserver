#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<err.h>
int main(int argc, char * argv[]){
  char inbuff[4097];
  int retnum = 0;
  if(argc < 3){
    fprintf(stderr, "improper args\n");
    return 22;
  }
  if(strlen(argv[1])> 1){
    fprintf(stderr, "multi char delim\n");
    return 2;
  }
  char delim = argv[1][0];
  int file = 0;
  int bytreadcnt = 4096;
  for(int i = 2; i < argc; i++){
     if(strcmp(argv[i], "-") == 0){
       do{
        bytreadcnt = read(0, inbuff, 4096);
	for(int i = 0; i < bytreadcnt; i++){
	   if(inbuff[i] == delim){
	     inbuff[i] = '\n';
	   }
	}
       // printf("%d\n", bytreadcnt);
        write(1, inbuff, bytreadcnt);
       }while(bytreadcnt >= 4096);
      
     }else{
         file = open(argv[i],O_RDONLY);
	 if(file <= 0){
	  // fprintf(stderr, "Failed to open file\n");
           retnum = 2;
	//     err(1, "couldnt open");
	 }
	 do{
         bytreadcnt = read(file, inbuff, 4096);
        
	for(int i = 0; i < bytreadcnt; i++){
	   if(inbuff[i] == delim){
	     inbuff[i] = '\n';
	   }
	}

     //          printf("%d\n", bytreadcnt);
        write(1, inbuff, bytreadcnt);
       }while(bytreadcnt >= 4096);
	close(file);

     }
  }
  return retnum;

}
