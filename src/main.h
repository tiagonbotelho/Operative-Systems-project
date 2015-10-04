#pragma once

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

#include "dnsserver.h"

#define TRUE 1
#define FALSE 0

void start_config();
void config();
void start_statistics();
void statistics();
