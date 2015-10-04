#include "main.h"

config_struct* get_configs(char* path){
  config_struct *var = malloc(sizeof(config_struct)); 
  FILE* file = fopen(path,"r");
  fscanf(file,"Threads = %d\n",&var->n_threads);
  return var;
}
