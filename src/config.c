#include "main.h"

//Creates a config_struct and fills it from path
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
  fscanf(file,"LocalDomain = %s\n",config->localdomain);
  fscanf(file,"NamedPipeEstatisticas = %s\n",config->pipename);
  fclose(file);
}
