#include "main.h"


//Gets info from config file and updates the config shared memory
void update_config(char* path){
    sem_wait(config_mutex);
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
    sem_post(config_mutex);
}
