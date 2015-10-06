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

void run_config(){
  printf("Started config process\n");
  update_config("../data/config.txt");
  printf("%d\n",config->n_threads);
  int i;
  printf("Domains:\n");
  for(i=0;i<2;i++){
    printf("%d:%s\n",i,config->domains[i]);
  }
  printf("Local Domain: %s\n",config->local_domain);
  printf("PipeName: %s\n",config->pipe_name);
}

void create_shared_memory(){
  configshmid = shmget(IPC_PRIVATE,sizeof(config_struct),IPC_CREAT|0700);
  config = (config_struct*)shmat(configshmid,NULL,0);
  update_config("../data/config.txt");
}

void delete_shared_memory(){
  shmctl(configshmid,IPC_RMID,NULL);
}

void start_config(){
  if(fork()==0){
    run_config();
    exit(0);
  }
}

int main(int argc, char const *argv[]){
  create_shared_memory();
  start_config();
  start_statistics();
  request_manager(argc,argv);
  delete_shared_memory();
  wait(NULL);
  return 0;
}


