#include "main.h"

/* It still needs some refining */
domain_struct parse_line(char *line) {
    int i = 0;
    int j = 0;
    domain_struct aux; 
    
    while (line[i] != ' ') {
      aux.dns[i] = line[i];
      i++;
    }
    aux.dns[i] = '\0';
    i++;

    while (line[i] != '\0') {
        aux.ip[j] = line[i];
        i++;
        j++;
    }
    aux.ip[j] = '\0';

    return aux;
}

void mem_mapped_file_init(char *path) {
    int fd;
    /*int i;*/
    
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file");
        exit(1);
    }
    addr = mmap(0, FILE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        printf("Error mapping the file");
        close(fd);
        exit(1);
    }

    close(fd);
}

int parse_dns_line(char *line, char *dns) {
  int i = 0;

  while (line[i] != ' ' && dns[i] != '\0') {
      printf("line: %c dns: %c\n", line[i], dns[i]);
      if (line[i] != dns[i]) {
          return -1;
      }
      i++;
  }
  return i;
}

char *find_local_mmaped_file(char *dns) {
    int length;
    char *line;
    char *aux = strdup(addr);
    char *ip;
    line = strtok(aux, "\n");

    /* While there are new lines */
    while (line != NULL) {
        printf("%s\n", line);
        if ((length = parse_dns_line(line, dns)) > -1) {
            printf("ENTREI AQUI CARALHO %lu %lu [%c]\n", strlen(line) - length - 1, strlen(line), line[length]);
            
            memcpy(ip, line + length+1, strlen(line)-length);
            printf("IP: %s\n", ip);
            return ip;
        }
        line = strtok(NULL, "\n");
    }

    return NULL;
}

void mem_mapped_file_terminate() {
    if (munmap(addr, FILE_SIZE) == -1) {
        perror("Error while unmapping file");
    }
}
