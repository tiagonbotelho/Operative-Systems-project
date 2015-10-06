
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/wait.h>

//Constants
#define N_DOMAINS 2
#define MAX_DOMAIN_CHARS 10
#define MAX_PIPE_NAME 15

#define TRUE 1
#define FALSE 0


//Structs
typedef struct config{
  int n_threads;
  char domains[N_DOMAINS][MAX_DOMAIN_CHARS];
  char localdomain[MAX_DOMAIN_CHARS];
  char pipename[MAX_PIPE_NAME];
} config_struct;

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


//Main.c
void start_config();
void run_config();
void start_statistics();
void statistics();

//Config.c
void update_config(char* path);

//Dnsserver.c
int request_manager(int argc ,const char *argv[]);
void sendReply(unsigned short id, unsigned char* query, int ip_addr, int sockfd, struct sockaddr_in dest);
u_char* convertRFC2Name(unsigned char* reader,unsigned char* buffer,int* count);
void convertName2RFC(unsigned char* dns,unsigned char* host);
int get_size(char* dns);
int is_local(char* dns, char* local);

//Global variables
int configshmid;
config_struct *config;
