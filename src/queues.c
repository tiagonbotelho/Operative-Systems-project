#include"main.h"

dnsrequest pop(dns_queue **top) {
    dnsrequest item;
    dns_queue *tmp;

    item = (*top)->request;
    tmp = *top;
    *top = (*top)->next_node;
    free(tmp);
    return item;
}

void push(dnsrequest item, dns_queue **top) {
    dns_queue *tmp;
    tmp = (dns_queue*)malloc(sizeof(dns_queue));
    tmp->request = item;
    tmp->next_node = *top;
    *top = tmp;
}

int stack_empty(dns_queue *tmp) {
    if (tmp == NULL) {
        return 1;
    } else {
        return 0;
    }
}

void schedule_request(int queue,short dns_id, int sockfd, unsigned char *dns_name, struct sockaddr_in dest){
    dnsrequest request;
    memcpy(&request.dest,&dest,sizeof(struct sockaddr_in));
    memcpy(&request.dns_name,dns_name,sizeof(char)*IP_SIZE);
    request.dns_id = dns_id;
    request.sockfd = sockfd; 
    if (queue == LOCAL) {
	push(request, &queue_local);
    } else {
	push(request, &queue_remote);
    }
}

dnsrequest get_request(int queue){
  if (queue == LOCAL) {
      return pop(&queue_local);
  }
  return pop(&queue_remote);
}

