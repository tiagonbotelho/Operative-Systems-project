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
    int i;

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
    for (i = 0; addr[i] != '\0'; i++) {
        if (addr[i] == '\n') {
            printf("NEW LINE\n");
        } else {
            printf("%d: %c\n", i, addr[i]);
        }
    }

    close(fd);
}

void mem_mapped_file_terminate() {
    if (munmap(addr, FILE_SIZE) == -1) {
        perror("Error while unmapping file");
    }
}
