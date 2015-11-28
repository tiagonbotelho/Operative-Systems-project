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

int parse_dns_line(char *line,char *dns) {
    int i = 0;

    while (line[i] != ' ' && dns[i] != '\0') {
	if (line[i] != dns[i]) {
	    return -1;
	}
	i++;
    }
    return i;
}

char *copy_string(char *line, int length) {
    int j;
    int i = 0;
    char *string = (char*)malloc(sizeof(char) * (strlen(line) - length));

    for (j = length; line[j] != '\0'; j++) {
	string[i] = line[j];
	i++;
    }
    string[i] = '\0';
    return string;
}


char *find_local_mmaped_file(char *dns) {
    int length;
    char *line;
    char *aux = strdup(addr);
    char *ip;
    line = strtok(aux, "\n");

    /* While there are new lines */
    while (line != NULL) {
    if ((length = parse_dns_line(line, dns)) > -1) {

            ip = copy_string(line, (length + 1));
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
