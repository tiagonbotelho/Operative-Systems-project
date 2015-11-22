#include"main.h"

stats_struct initialize_stats(){
  stats_struct stats;
  time_t rawtime;
  time (&rawtime);
  stats.start_time = localtime ( &rawtime );
  stats.requests_denied = 0;
  stats.local_domains_resolved = 0;
  stats.extern_domains_resolved = 0;
  stats.last_time = localtime( &rawtime);
  return stats;
}

void print_stats(){
  char *pipe_name = (char *)malloc(MAX_PIPE_NAME);
  stats_struct stats;
  sem_wait(config_mutex);
  strcpy(pipe_name,config->pipe_name);
  sem_post(config_mutex);  
  printf("PIPE %s\n",pipe_name);
  int fd = open(pipe_name,O_RDONLY);
  read(fd,&stats,sizeof(stats_struct));
  printf ( "Server start time: %s", asctime (stats.start_time) );
  close(fd);
}
