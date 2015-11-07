#include "main.h"

pthread_mutex_t mutex_thread = PTHREAD_MUTEX_INITIALIZER;

void statistics() {
  printf("Started statistics process\n");
}

void start_statistics() {
  if(fork()==0){
    printf("Stats pid = %lu",(long)getpid());
    statistics();
    exit(0);
  }
}

void run_config() {
  int i;
  printf("Started config process\n");
  update_config("../data/config.txt");
  printf("%d\n",config->n_threads);
  printf("Domains:\n");
  for(i = 0; i < MAX_N_DOMAINS; i++) {
    printf("%d:%s\n",i,config->domains[i]);
  }
  printf("Local Domain: %s\n",config->local_domain);
  printf("PipeName: %s\n",config->pipe_name);
}

void create_ip_list() {

}

void create_shared_memory() {
  create_ip_list();
  configshmid = shmget(IPC_PRIVATE,sizeof(config_struct),IPC_CREAT|0700);
  config = (config_struct*)shmat(configshmid,NULL,0);
  update_config("../data/config.txt");  
  printf("OLA\n");
}

void delete_shared_memory() {
  shmctl(configshmid,IPC_RMID,NULL);
}

void start_config() {
  if(fork()==0){
    printf("Config pid = %lu",(long)getpid());
    run_config();
    exit(0);
  }
}

void create_semaphores() {
  sem_unlink("CONFIG_MUTEX");
  config_mutex = sem_open("CONFIG_MUTEX",O_CREAT|O_EXCL,0700,1);
}

void *thread_behaviour(void *args) {
    /*dnsrequest request = get_request(priority_queue);*/
    pthread_mutex_lock(&mutex_thread);
    printf("Thread is writing...\n");
    /*if (request.mtype == 2)  {*/
    printf("LOCAL IP: %s\n", find_local_mmaped_file("fileserver.so.local"));
    /*} */
    printf("Thread sleeping");
    pthread_mutex_unlock(&mutex_thread);
    
    pthread_exit(NULL);
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

void delete_semaphores() {
  sem_close(config_mutex);
  sem_unlink("CONFIG_MUTEX");
}

void sigint_handler() {
    /* TODO: Escrever para estatisticas */
    terminate();
    printf("Thank you! Shutting Down\n");
    exit(1);
}

/* Initializes semaphores shared mem config statistics and threads */
void init() {
    mem_mapped_file_init("../data/localdns.txt");
    requests_queue = msgget(IPC_PRIVATE, IPC_CREAT|0700);
    create_semaphores();
    create_shared_memory();
    start_config();
    start_statistics();
    create_threads();
}

/* Terminate processes shared_memory and semaphores */
void terminate() {
    int i;
    for (i = 0; i < 2; i++) {
        wait(NULL);
    }
    msgctl(requests_queue, IPC_RMID, NULL);
    delete_shared_memory();
    delete_semaphores();
    mem_mapped_file_terminate();
}

int main(int argc, char const *argv[]) {
  if(argc <= 1) {
    printf("Usage: dnsserver <port>\n");
    exit(1);
  }
  signal(SIGINT, sigint_handler);
  init();
  request_manager(atoi(argv[1]));
  terminate();
  return 0;
}


