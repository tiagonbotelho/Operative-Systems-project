#include"main.h"

void schedule_request(int queue,char *ip, struct sockaddr_in dest){
  dnsrequest request;
  request.mtype = 1;
  memcpy(&request.dest,&dest,sizeof(struct sockaddr_in));
  memcpy(&request.ip,ip,sizeof(char)*IP_SIZE);
  msgsnd(queue,&request,sizeof(dnsrequest)-sizeof(long),0);
}

dnsrequest get_request(int queue){
  dnsrequest request;
  msgrcv(queue,&request,sizeof(dnsrequest)-sizeof(long),1,0);
  printf("TEST - %s\n",request.ip);
  return request;
}
