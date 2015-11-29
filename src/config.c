#include "main.h"

//Gets info from config file and updates the config shared memory
void update_config(char* path){
    FILE* file = fopen(path,"r");
    char aux;
    fscanf(file,"Threads = %d\n",&config->n_threads);
    fseek(file,strlen("Domains = "),SEEK_CUR);
    int current_domain = 0;
    int current_pos =0;
    while((aux = fgetc(file)) != '\n'){
	if(aux==';'){
	    config->domains[current_domain][current_pos] = '\0';
	    current_domain++;
	    current_pos = 0;
	    continue;
	}
	if(aux==' '){
	    continue;
	}
	config->domains[current_domain][current_pos++] = aux;
    }
    fscanf(file,"LocalDomain = %s\n",config->local_domain);
    fscanf(file,"NamedPipeEstatisticas = %s\n",config->pipe_name);
    fclose(file);
}

void terminate_config(){
    exit(0);
}

void config_updated(){
    sem_wait(wait_for_config);
    memset(config,0,sizeof(config_struct));
    update_config("../data/config.txt");
    sem_post(wait_for_config);
    printf("Config updated\n");
    sem_wait(in_maintenance_mutex);
    *in_maintenance = FALSE;
    sem_post(in_maintenance_mutex);
}
void sighup_handle(){
    printf("Started maintenance mode. Only local domains are being handled\n");
    sem_wait(in_maintenance_mutex);
    *in_maintenance = TRUE;
    sem_post(in_maintenance_mutex);
    sigset_t set;
    sigemptyset(&set);
    signal(SIGUSR1,config_updated);
    sigsuspend(&set);
}


void run_config() {
    signal(SIGINT,terminate_config);
    printf("Started config process\n");
    update_config("../data/config.txt");
    sem_post(wait_for_config);

    sigset_t set;
    while(1){
	sigemptyset(&set);
	signal(SIGUSR1,sighup_handle);
	sigsuspend(&set);
    }
}
