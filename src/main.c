#include "main.h"

void statistics(){
  printf("Started statistics process\n");
  while(1){
    
  }
}

void start_statistics(){
  if(fork()==0){
    statistics();
    exit(0);
  }
}

void config(){
  printf("Started config process\n");
  config_struct *config = get_configs("../data/config.txt");
  printf("%d\n",config->n_threads);
}


void start_config(){
  if(fork()==0){
    config();
    exit(0);
  }
}

int main(int argc, char const *argv[]){
  start_config();
  start_statistics();
  request_manager(argc,argv);
  return 0;
}


