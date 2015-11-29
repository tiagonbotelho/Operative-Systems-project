#include"main.h"

stats_struct initialize_stats(){
  stats_struct stats;
  stats.requests_denied = 0;
  stats.local_domains_resolved = 0;
  stats.extern_domains_resolved = 0;
  return stats;
}

void print_stats(){
    pthread_mutex_lock(&stats_mutex);
    int sum = stats.requests_denied + stats.extern_domains_resolved + stats.local_domains_resolved;
    printf("Server start time: ");
    print_time_instant(stats.start_time);
    printf("\nLocal domains resolved: %d\nExtern domains resolved:%d\nRequests refused: %d\nTotal requests received: %d\nLast information received: ",stats.local_domains_resolved,stats.extern_domains_resolved,stats.requests_denied,sum);
    print_time_instant(stats.last_time);
    printf("\n");
    pthread_mutex_unlock(&stats_mutex);
}

void terminate_stats(){
    pthread_kill(reader,SIGKILL);
    exit(1);
}

void statistics() {
    signal(SIGINT,terminate_stats);
    printf("Started statistics process\n");
    stats = initialize_stats();
    int start_time_pipe = open(config->pipe_name, O_RDONLY);
    if(start_time_pipe<0){
	perror("Erro");
    }
    time_instant start_time;
    read(start_time_pipe,&start_time,sizeof(time_instant));
    stats.start_time = start_time;
    stats.last_time = start_time;
    pthread_create(&reader, NULL,reader_code,NULL);
    while(TRUE){
	print_stats();
	sleep(30);
    }
    close(start_time_pipe);
}

void print_time_instant(time_instant time){
    printf("%d/%d/%d %d:%d:%d",time.day,time.month,time.year,time.hour,time.minute,time.seconds);
}

void *reader_code(void* args){
    char aux;
    int read_fd = open(config->pipe_name,O_RDONLY);
    while(1){
	read(read_fd,&aux,sizeof(char));
	pthread_mutex_lock(&stats_mutex);
	if(aux == 'l'){
	    stats.local_domains_resolved++;
	}
	else if(aux == 'e'){
	    stats.extern_domains_resolved++;
	}
	else if(aux == 'd'){
	    stats.requests_denied++;
	}
	stats.last_time = get_current_time();
	pthread_mutex_unlock(&stats_mutex);
    }
}
