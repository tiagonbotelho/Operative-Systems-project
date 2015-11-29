#include "main.h"

int request_manager(int port) 
{
    signal(SIGINT, sigint_handler);
    printf("Started request manager process\n");
    unsigned char buf[65536], *reader;
    int stop;
    struct DNS_HEADER *dns = NULL;
    struct sockaddr_in dest;
    socklen_t len;
    fd = open(config->pipe_name,O_WRONLY);
    // ****************************************
    // Receive questions
    // ****************************************
    while(1) {
	// Receive questions
        len = sizeof(dest);
        printf("\n\n-- Wating for DNS message --\n\n");
        if (recvfrom (sockfd,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , &len) < 0) {
            printf("Error while waiting for DNS message. Exiting...\n");
            exit(1);
        }
	
        printf("DNS message received\n");

        // Process received message
        dns = (struct DNS_HEADER*) buf;
        reader = &buf[sizeof(struct DNS_HEADER)];
	
        printf("\nThe query %d contains: ", ntohs(dns->id));
        printf("\n %d Questions.", ntohs(dns->q_count));
        printf("\n %d Answers.", ntohs(dns->ans_count));
        printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
        printf("\n %d Additional records.\n\n", ntohs(dns->add_count));

        // We only need to process the questions
        // We only process DNS messages with one question
        // Get the query fields according to the RFC specification
        struct QUERY query;
        if (ntohs(dns->q_count) == 1) {
            // Get NAME
            query.name = convertRFC2Name(reader,buf,&stop);
            reader = reader + stop;

            // Get QUESTION structure
            query.ques = (struct QUESTION*)(reader);
            reader = reader + sizeof(struct QUESTION);

            // Check question type. We only need to process A records.
            if(ntohs(query.ques->qtype) == 1) {
                printf("A record request.\n\n");
            } else {
                printf("NOT A record request!! Ignoring DNS message!\n");
                continue;
            }

        } else {
            printf("\n\nDNS message must contain one question!! Ignoring DNS message!\n\n");
            continue;
        }

        // Received DNS message fulfills all requirements.
        
        // ****************************************
        // Print received DNS message QUERY
        // ****************************************
        printf(">> QUERY: %s\n", query.name);
        printf(">> Type (A): %d\n", ntohs(query.ques->qtype));
        printf(">> Class (IN): %d\n\n", ntohs(query.ques->qclass));
	
        // ****************************************
        // Example reply to the received QUERY
        // ****************************************
	sem_wait(wait_for_config);
	if(validate_local_domain((char *)query.name)){
	    pthread_mutex_lock(&local_buffer_mutex);
	    schedule_request(LOCAL,dns->id,sockfd,query.name,dest);
	    pthread_mutex_unlock(&local_buffer_mutex);
	    sem_post(n_requests);
	}
	else {
	    sem_wait(in_maintenance_mutex);
	    if(!*in_maintenance){
		pthread_mutex_lock(&remote_buffer_mutex);
		schedule_request(REMOTE,dns->id,sockfd,query.name,dest);
		pthread_mutex_unlock(&remote_buffer_mutex);
		sem_post(n_requests);
	    }
	    sem_post(in_maintenance_mutex);
	}
	sem_post(wait_for_config);
    }
    
    return 0;

}

/**
   sendReply: this method sends a DNS query reply to the client
   * id: DNS message id (required in the reply)
   * query: the requested query name (required in the reply)
   * ip_addr: the DNS lookup reply (the actual value to reply to the request)
   * sockfd: the socket to use for the reply
   * dest: the UDP package structure with the information of the DNS query requestor (includes it's IP and port to send the reply)
   **/
void sendReply(unsigned short id, unsigned char* query, int ip_addr, int sockfd, struct sockaddr_in dest) {
    unsigned char bufReply[65536], *rname;
    char *rip;
    struct R_DATA *rinfo = NULL;

    //Set the DNS structure to reply (according to the RFC)
    struct DNS_HEADER *rdns = NULL;
    rdns = (struct DNS_HEADER *)&bufReply;
    rdns->id = id;
    rdns->qr = 1;
    rdns->opcode = 0;
    rdns->aa = 1;
    rdns->tc = 0;
    rdns->rd = 0;
    rdns->ra = 0;
    rdns->z = 0;
    rdns->ad = 0;
    rdns->cd = 0;
    rdns->rcode = 0;
    rdns->q_count = 0;
    rdns->ans_count = htons(1);
    rdns->auth_count = 0;
    rdns->add_count = 0;

    // Add the QUERY name (the same as the query received)
    rname = (unsigned char*)&bufReply[sizeof(struct DNS_HEADER)];
    convertName2RFC(rname , query);

    // Add the reply structure (according to the RFC)
    rinfo = (struct R_DATA*)&bufReply[sizeof(struct DNS_HEADER) + (strlen((const char*)rname)+1)];
    rinfo->type = htons(1);
    rinfo->_class = htons(1);
    rinfo->ttl = htonl(3600);
    rinfo->data_len = htons(sizeof(ip_addr)); // Size of the reply IP address

    // Add the reply IP address for the query name 
    rip = (char *)&bufReply[sizeof(struct DNS_HEADER) + (strlen((const char*)rname)+1) + sizeof(struct R_DATA)];
    memcpy(rip, (struct in_addr *) &ip_addr, sizeof(ip_addr));

    // Send DNS reply
    printf("\nSending Answer... ");
    if( sendto(sockfd, (char*)bufReply, sizeof(struct DNS_HEADER) + (strlen((const char*)rname) + 1) + sizeof(struct R_DATA) + sizeof(ip_addr),0,(struct sockaddr*)&dest,sizeof(dest)) < 0) {
        printf("FAILED!!\n");
    } else {
        printf("SENT!!!\n");
    }
}

/**
   convertRFC2Name: converts DNS RFC name to name
**/
u_char* convertRFC2Name(unsigned char* reader,unsigned char* buffer,int* count) {
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name = (unsigned char*)malloc (256);

     name[0]='\0'; 

    while(*reader!=0) {
	if(*reader>=192) {
            offset = (*reader)*256 + *(reader+1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1;
        } else {
            name[p++]=*reader;
        }

        reader = reader+1;

        if(jumped==0) {
            *count = *count + 1;
        }
    }

    name[p]='\0';
    if(jumped==1) {
        *count = *count + 1;
    }

    for(i=0;i<(int)strlen((const char*)name);i++) {
        p=name[i];
        for(j=0;j<(int)p;j++) {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0';
    return name;
}

/**
   convertName2RFC: converts name to DNS RFC name
**/
void convertName2RFC(unsigned char* dns,unsigned char* host) {
    int lock = 0 , i;
    strcat((char*)host,".");

    for (i = 0 ; i < strlen((char*)host) ; i++) {
        if(host[i]=='.') {
            *dns++ = i-lock;
            for(;lock<i;lock++) {
                *dns++=host[lock];
            }
            lock++;
        }
    }
    *dns++='\0';
}

// Gets size of the char pointer
int get_size(char *dns) {
    int i = 0;

    while (dns[i] != '\0') {
        i++;
    }

    return i;
}

int compare_domains(char *to_compare,  char *comparable) {
    int size = strlen((char *)to_compare);
    int size_comp = strlen((char *)comparable);
    int i;
    
    for (i = 1; i <= size_comp; i++) {
        if (to_compare[size - i] != comparable[size_comp - i]) {
            return FALSE;
        }
    }

    return TRUE;
}

int validate_local_domain(char *dns) {
    return compare_domains(dns, config->local_domain);
}

int validate_remote_domain(char *dns) {
    int i = 0; 
	    
    while (config->domains[i][0] != '\0' && i < MAX_N_DOMAINS) {
	printf("%s\n",config->domains[i]);
	if (compare_domains(dns, config->domains[i]) == TRUE) {
            return TRUE;
        }
        i++;
    }

    return FALSE; 
}
