#include "main.h"

void statistics(){
  printf("Started statistics process\n");
}

void start_statistics(){
  if(fork()==0){
    statistics();
    exit(0);
  }
}

void run_config(){
  int i;
  printf("Started config process\n");
  update_config("../data/config.txt", getpid());
  printf("%d\n",config->n_threads);
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
  update_config("../data/config.txt",getpid());
  
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

void create_semaphores(){
  sem_unlink("CONFIG_MUTEX");
  config_mutex = sem_open("CONFIG_MUTEX",O_CREAT|O_EXCL,0700,1);
}

void *thread_behaviour(void *arg) {
    long n = (long)arg;
    printf("%ld is alive!\n", n);
    return NULL;
}

void create_threads() {
    int i;
    pthread_t thread_pool[config->n_threads];

    for (i = 0; i < config->n_threads; i++) {
        pthread_create(&thread_pool[i], NULL, thread_behaviour, (void*)((long)i));
    }

    for (i = 0; i < config->n_threads; i++) {
        printf("thread %d shutting down\n", i);
        pthread_join(thread_pool[i], NULL);
    }
}

void delete_semaphores(){
  sem_close(config_mutex);
  sem_unlink("CONFIG_MUTEX");
}

int main(int argc, char const *argv[]){
  create_semaphores();
  create_shared_memory();
  start_config();
  start_statistics();
  create_threads();
  request_manager(argc,argv);
  int i;
  for(i=0;i<2;i++)
    wait(NULL);
  delete_shared_memory();
  delete_semaphores();
  return 0;
}


