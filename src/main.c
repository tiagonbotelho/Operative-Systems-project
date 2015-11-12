#include "main.h"


void statistics() {
  create_pipe();
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

void send_reply(dnsrequest request, char *ip) {
    sendReply(request.dns_id, request.dns_name, inet_addr(ip), request.sockfd, request.dest);
}

void handle_remote(char dns_name[]){
  char *command = (char *)malloc(sizeof("dig ")+ sizeof(dns_name) + 1);
  strcpy(command,"dig ");
  strcat(command,dns_name);
  FILE *in;
  char buff[512];
  if(!(in = popen(command, "r"))){
    terminate();
  }
  while(fgets(buff, sizeof(buff), in)!=NULL){
    printf("%s",buff);
  }
  close(in);
}

void *thread_behaviour(void *args) {
    dnsrequest request;
    char *request_ip;
    while(1){
        printf("Thread %lu is locked\n",(long)args);
        pthread_cond_wait(&cond_thread,&mutex_thread);
        printf("Thread %lu is writing...\n",(long)args);
        request = get_request(LOCAL);

        if (request.dns_id == -1) {
            dnsrequest aux_request = get_request(REMOTE);
            if (aux_request.dns_id == -1) {
                printf("ola\n");
                send_reply(aux_request, "0.0.0.0");
            } else {;
		handle_remote(aux_request.dns_name);
            }
        } else {
            if ((request_ip = find_local_mmaped_file(request.dns_name)) != NULL) {
                send_reply(request, request_ip);
            } else {
                send_reply(request, "0.0.0.0");
            }
        }

        printf("Thread sleeping");
        pthread_mutex_unlock(&mutex_thread);
    }
  pthread_exit(NULL);
  return NULL;

}

void create_threads() {
    int i;
    pthread_t thread_pool[config->n_threads];
    pthread_mutex_init(&mutex_thread,NULL);
    pthread_cond_init(&cond_thread,NULL);
    for (i = 0; i < config->n_threads; i++) {
      pthread_create(&thread_pool[i], NULL, thread_behaviour, (void*)((long)i));
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



void create_socket(int port){
  
    struct sockaddr_in servaddr;
    // Get server UDP port number
    if(port <= 0) {
        printf("Usage: dnsserver <port>\n");
        exit(1);
    }

    // Create UDP socket
    sockfd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries

    // If failed to open socket
    if (sockfd < 0) {
        printf("ERROR opening socket.\n");
        exit(1);
    }

    // Prepare UDP to bind port
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
    
    // Bind application to UDP port
    int res = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    // Failure on association of application to UDP port
    if(res < 0) {
      printf("Error binding to port %d.\n", servaddr.sin_port);

        if(servaddr.sin_port <= 1024) {
            printf("To use ports below 1024 you may need additional permitions. Try to use a port higher than 1024.\n");
        } else {
            printf("Please make sure this UDP port is not being used.\n");
        }
        exit(1);
   } 
}

void create_pipe(){
  unlink(config->pipe_name);
  sem_wait(config_mutex);
  if(mkfifo(config->pipe_name,O_CREAT|O_EXCL|0600)<0){
    perror("Cannot create pipe: ");
    exit(0);
  }
  sem_post(config_mutex);
}

/* Initializes semaphores shared mem config statistics and threads */
void init(int port) {
    create_semaphores();
    create_shared_memory();
    start_config();
    create_pipe();
    start_statistics();
    create_threads();    
    mem_mapped_file_init("../data/localdns.txt");
    requests_queue = msgget(IPC_PRIVATE, IPC_CREAT|0700);
    create_socket(port);
    
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
  init(atoi(argv[1]));
  request_manager();
  terminate();
  return 0;
}


