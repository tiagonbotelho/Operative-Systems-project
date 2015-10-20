#include"main.h"

void schedule_request(int queue,struct QUERY query, struct sockaddr_in dest){
  dnsrequest request;
  memcpy(&query, &request.query,sizeof(struct QUERY));
  memcpy(&dest, &request.dest,sizeof(struct sockaddr_in));
  msgsnd(queue,&request,sizeof(dnsrequest)-sizeof(long),0);
}

dnsrequest get_request(int queue){
  dnsrequest request;
  msgrcv(queue,&request,sizeof(dnsrequest)-sizeof(long),0,0);
  return request;
}
