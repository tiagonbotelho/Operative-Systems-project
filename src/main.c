#include "main.h"

void statistics(){
  while(1){
    printf("STATISTICS PROCESS");
  }
}

void start_statistics(){
  if(fork()==0){
    statistics();
  }
}

void config(){
  while(1){
    printf("CONFIG PROCESS");
  }
}


void start_config(){
  if(fork()==0){
    config();
  }
}

int main(int argc, char const *argv[]){
  start_config();
  config();
  request_manager(argc,argv);
  return 0;
}


