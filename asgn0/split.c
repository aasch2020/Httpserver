#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
int main(int argc, char * argv[]){
  char inbuff[4096];
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
        printf("%d\n", bytreadcnt);
        write(1, inbuff, bytreadcnt);
       }while(bytreadcnt == 4096);
      
     }else{
         file = open(argv[i],O_RDONLY);
	 do{
         bytreadcnt = read(file , inbuff, 4096);

	for(int i = 0; i < bytreadcnt; i++){
	   if(inbuff[i] == delim){
	     inbuff[i] = '\n';
	   }
	}

               printf("%d\n", bytreadcnt);
        write(1, inbuff, bytreadcnt);
       }while(bytreadcnt == 4096);

     }
  }
}
