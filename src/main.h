#include<semaphore.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ipc.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/fcntl.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/msg.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>

//Constants
#define MAX_N_DOMAINS 2
#define MAX_DOMAIN_CHARS 10
#define MAX_PIPE_NAME 15
#define IP_SIZE 100
#define DNS_SIZE 100
#define MAX_LOCAL 100
#define NUMINTS 1000
#define FILE_SIZE NUMINTS*sizeof(char)

#define TRUE 1
#define FALSE 0

#define LOCAL 2 //This numbers because mtype can't be 0
#define REMOTE 1 

//Structs
typedef struct config{
  int n_threads;
  char domains[MAX_N_DOMAINS][MAX_DOMAIN_CHARS];
  char local_domain[MAX_DOMAIN_CHARS];
  char pipe_name[MAX_PIPE_NAME];
} config_struct;

typedef struct stats{
  struct tm *start_time;
  int requests_denied;
  int local_domains_resolved;
  int extern_domains_resolved;
  struct tm *last_time;
} stats_struct;

typedef struct domain {
    char ip[IP_SIZE];
    char dns[DNS_SIZE];
} domain_struct;

//DNS header structure
struct DNS_HEADER
{
    unsigned short id; // identification number

    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};

//Structure of a Query
struct QUERY
{
    unsigned char *name;
    struct QUESTION *ques;
};

typedef struct request {
  unsigned char dns_name[IP_SIZE];
  short dns_id;
  int sockfd;
  struct sockaddr_in dest;
} dnsrequest;

typedef struct dnsqueue {
    dnsrequest request;
    struct dnsqueue *next_node;
} dns_queue;

//Main.c
void start_config();
void run_config();
void start_statistics();
void statistics();
void terminate();
void create_pipe();


//Queues.c
dnsrequest get_request(int queue);
void schedule_request(int queue,short dns_id, int sockfd, char *ip, struct sockaddr_in dest);
dns_queue *get_node(dnsrequest item);
int stack_empty(dns_queue *tmp);
dnsrequest pop(dns_queue **top);
void push(dnsrequest item, dns_queue **top);


//Config.c
void update_config(char* path);

//mmapped_file.c
domain_struct parse_line(char * line);
void mem_mapped_file_init(char *path);
void mem_mapped_file_terminate();
char *find_local_mmaped_file(unsigned char *dns);
int parse_dns_line(unsigned char *line, unsigned char *dns);


//Dnsserver.c
int request_manager();
void sendReply(unsigned short id, unsigned char* query, int ip_addr, int sockfd, struct sockaddr_in dest);
u_char* convertRFC2Name(unsigned char* reader,unsigned char* buffer,int* count);
void convertName2RFC(unsigned char* dns,unsigned char* host);
int compare_domains(unsigned char *to_compare, unsigned char *comparable);
int validate_local_domain(unsigned char *dns);
int validate_remote_domain(unsigned char *dns);

//Stats.c
void print_stats();
stats_struct initialize_stats();

//Global variables
int configshmid; //shared memory id to configs
config_struct *config; //config structure in shared memory
sem_t *config_mutex; //mutex to prevent racing in config
char *addr; //Address that contains mmapped_file information
int requests_queue; //message queue for requests
int sockfd; //Socket that receives requests
pthread_mutex_t mutex_thread; //Temporary mutex for threads
pthread_cond_t cond_thread; //Temporary conditional variable
dns_queue *queue_local; // Queue of local requests
dns_queue *queue_remote; // Queue of remote requests
