#include "main.h"

void statistics(){
  printf("Started statistics process\n");
  while(1){
    
  }
}

void start_statistics(){
  if(fork()==0){
    statistics();
  }
}

void config(){
  printf("Started config process\n");
  while(1){
  }
}


void start_config(){
  if(fork()==0){
    config();
  }
}

int main(int argc, char const *argv[]){
  start_config();
  start_statistics();
  request_manager(argc,argv);
  return 0;
}


