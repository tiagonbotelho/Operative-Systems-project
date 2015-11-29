#include "main.h"

void start_statistics() {
    statistics_pid = fork();
    if(statistics_pid==0){
        printf("Stats pid = %lu\n",(long)getpid());
        statistics();
        exit(0);
    }
    else if(statistics_pid<0){
	perror("Error creating stats process");
	exit(1);
    }
}

void terminate_config(){
    exit(0);
}

void run_config() {
    signal(SIGINT,terminate_config);
    printf("Started config process\n");
    update_config("../data/config.txt");
    sem_post(wait_for_config);
}


void create_shared_memory() {
    if((configshmid = shmget(IPC_PRIVATE,sizeof(config_struct),IPC_CREAT|0700)) == -1){
	perror("Error creating shared memory segment");
	exit(1);
    }
    if((config = (config_struct*)shmat(configshmid,NULL,0))== (void*)-1){
	perror("Error attaching shared memory segment to the address space");
	exit(1);
    }
}

void delete_shared_memory() {
    shmctl(configshmid,IPC_RMID,NULL);
}

void start_config() {
    config_pid = fork();
    if(config_pid == 0){
        printf("Config pid = %lu\n",(long)getpid());
        run_config();
        exit(0);
    } else if(config_pid<0){
	perror("Error creating config process");
	exit(1);
    }
}

void create_semaphores() {
    pthread_mutex_init(&stats_mutex,NULL);	
    sem_unlink("WAIT_FOR_CONFIG");
    if((wait_for_config = sem_open("WAIT_FOR_CONFIG",O_CREAT|O_EXCL,0700,0)) == SEM_FAILED){
	perror("Error initializing n_requests semaphore");
	exit(1);
    }
    sem_unlink("N_REQUESTS");
    if((n_requests  = sem_open("N_REQUESTS",O_CREAT|O_EXCL,0700,0)) == SEM_FAILED){
	perror("Error initializing n_requests semaphore");
	exit(1);
    }
}

void send_reply(dnsrequest request, char *ip) {
    sendReply(request.dns_id, (unsigned char*)request.dns_name, inet_addr(ip), request.sockfd, request.dest);
}

int handle_remote(dnsrequest request) {
    char *command = (char *)malloc(sizeof("dig +short ")+ sizeof(request.dns_name) + 1);
    char *ip = (char*)malloc(IP_SIZE);
    strcpy(command,"dig +short ");
    strcat(command,request.dns_name);
    FILE *in;
    char buff[512];
    if(!(in = popen(command, "r"))){
        terminate();
    }
    while (fgets(buff, sizeof(buff), in) != 0 ) {
        if (isdigit(buff[0])) {
            strncpy(ip, buff, strlen(buff));
            ip[strlen(ip)-1] = '\0';
            send_reply(request, ip);
            return TRUE;
        }
    }
    return FALSE;
}

void terminate_thread(){
    printf("Thread morreu\n");
    pthread_exit(0);
}

void *thread_behaviour(void *args) {
    signal(SIGUSR1,terminate_thread);
    dnsrequest request;
    char *request_ip;
    while(1){
        printf("Thread %lu is locked\n",(long)args);
	sem_wait(n_requests);
	printf("Thread %lu is writing...\n",(long)args);
	char aux;
	if (stack_empty(queue_local) == 0) {
	    request = get_request(LOCAL);
            if ((request_ip = find_local_mmaped_file(request.dns_name)) != NULL) {
                send_reply(request, request_ip);
		aux = 'l';
            } else {
                send_reply(request, "0.0.0.0");
		aux = 'd';
	    }

        } else if (stack_empty(queue_remote) == 0) {
	    request = get_request(REMOTE);
	    if(handle_remote(request)){
		aux = 'e';
	    }else{
		send_reply(request, "0.0.0.0");
		aux = 'd';
	    }
        }
	write(fd,&aux,sizeof(char));
	printf("Thread sleeping");
	
    }
    pthread_exit(NULL);
    return NULL;

}

void create_threads() {
    int i;
    if((thread_pool = malloc(sizeof(pthread_t)*config->n_threads)) == NULL){
	perror("Error allocating memory for thread pool");
    }
    int n = config->n_threads;
    for (i = 0; i < n; i++) {
        pthread_create(&thread_pool[i], NULL, thread_behaviour, (void*)((long)i));	
    }
}

void delete_semaphores() {
    sem_close(n_requests);
    sem_unlink("N_REQUESTS");
    sem_close(wait_for_config);
    sem_unlink("WAIT_FOR_CONFIG");
}

void sigint_handler() {
    printf("Thank you! Shutting Down\n");
    for(int i=0;i<config->n_threads;i++){
	pthread_kill(thread_pool[i],SIGUSR1);
    }
    terminate();
    exit(1);
}

void create_socket(int port){
    struct sockaddr_in servaddr; 
    // Create UDP socket
    sockfd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries

    // If failed to open socket
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Prepare UDP to bind port
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);

    // Bind application to UDP nnport
    int res = bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    // Failure on association of application to UDP port
    if(res < 0) {
        printf("Error binding to port %d.\n", servaddr.sin_port);

        if(servaddr.sin_port <= 1024) {
            printf("To use ports below 1024 you may need additional permitions. Try to use a port higher than 1024.\n");
        } else {
            printf("Please make sure this UDP port is not being used.\n");
        }
	terminate();
        exit(1);
    } 
}

void create_pipe(){
    unlink(config->pipe_name);
    if(mkfifo(config->pipe_name,O_CREAT|O_EXCL|0600)<0){
	perror("Cannot create pipe: ");
	exit(0);
    }
    printf("Created pipe\n");
}

/* Initializes semaphores shared mem config statistics and threads */
void init(int port) {
    create_semaphores();
    create_shared_memory();
    start_config();
    sem_wait(wait_for_config);
    start_statistics();
    create_pipe();
    create_threads();
    mem_mapped_file_init("../data/localdns.txt");
    create_socket(port);
    send_start_time_to_pipe();
    if((queue_local = (dns_queue*)malloc(sizeof(dns_queue)))==NULL){
	perror("Error allocating local queue");
    }
    if((queue_remote = (dns_queue*)malloc(sizeof(dns_queue)))==NULL){
	perror("Error allocating remote queue");
    }
    queue_local = NULL;
    queue_remote = NULL;
}

time_instant get_current_time(){
    time_t rawtime;
    time (&rawtime);
    struct tm *current_time = localtime ( &rawtime );
    time_instant now;
    now.hour = current_time->tm_hour;
    now.minute = current_time->tm_min;
    now.seconds = current_time->tm_sec;
    now.day = current_time->tm_mday;
    now.month = current_time->tm_mon +1;
    now.year = current_time->tm_year + 1900;
    return now;
}

void send_start_time_to_pipe(){
    int start_time_pipe= open(config->pipe_name,O_WRONLY);
    time_instant now = get_current_time();
    write(start_time_pipe,&now,sizeof(time_instant));
    close(start_time_pipe);
}

/* Terminate processes shared_memory and semaphores */
void terminate() {
    printf("Processes killed\n");
    unlink(config->pipe_name);
    mem_mapped_file_terminate();
    delete_shared_memory();
    delete_semaphores();
    printf("TESTE\n");
}

int main(int argc, char const *argv[]) {
    if(argc <= 1) {
        printf("Usage: dnsserver <port>\n");
        exit(1);
    }
    int port = atoi(argv[1]);
    init(port);
    request_manager();
    terminate();
    return 0;
}














