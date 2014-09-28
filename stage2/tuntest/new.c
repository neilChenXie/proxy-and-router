/*
 *proxy.c 
 writen by Chen Xie
 Date: 09/24/2014
 v0.1
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <net/if.h>
#include <func.h>

#define MAXROUTER 10 //for router_port array initial

#define MAXBUFLEN 100

