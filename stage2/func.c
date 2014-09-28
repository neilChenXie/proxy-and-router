#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "func.h"

int num_stage = 0;
int num_router = 0;
int proxy_sockfd;
int proxy_port;
int router_sockfd;
int router_port;
int rec_router_port[MAXROUTER];

/***************read file functons*************/
/*return the num of stage*/
int stage_line(char *sp) {
	char *comm = "#";
	char *stage = "stage";
	char *startp;
	//char *sh;

	if (strstr(sp,comm) != NULL) {
		printf("comment_line: %s\n", sp);
		return 0;
	} else {
		if (strstr(sp, stage) != NULL) {
			/*get stage num*/
			startp = strchr(sp, ' ');
			startp++;//point to the num
			printf("stage_line: %s\n", startp);
			return atoi(startp);
		}
		return -1;
	}
}

/*return the num of router*/
int router_line(char *sp) {
	char *comm = "#";
	char *routnum = "num_routers";
	char *startp;
	//char *sh;

	if (strstr(sp,comm) != NULL) {
		printf("comment_line: %s\n", sp);
		return 0;
	} else {
		if (strstr(sp, routnum) != NULL) {
			/*get stage num*/
			startp = strchr(sp, ' ');
			startp++;//point to the num
			printf("router_line: %s\n", startp);
			return atoi(startp);
		}
		return -1;
	}
}
/*read config*/
int read_config(FILE *fp) {
	char line[LINELEN];
	int rv;
	if (fp == NULL) {
		return -1;
	}
	while(fgets(line, sizeof(line), fp) != NULL &&(num_stage == 0 || num_router == 0)) {
		if (num_stage == 0) {
			rv = stage_line(line);
			if(rv > 0) {
				num_stage = rv;
			}
			continue;
		} else {
			/*num of routers*/
			rv = router_line(line);
			if(rv > 0) {
				num_router = rv;
			}
			continue;
		}
	}
	if(num_stage == 0 || num_router == 0) {
		return -1;
	}
	//printf("stage %d\n", num_stage);
	//printf("num_router %d\n", num_router);
	return 0;
}
/*************************************/

/**************get addrinfo***********/
/*return base on IPv4/IPv6 judgement*/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*get port num*/
unsigned short get_port(struct sockaddr *sa) 
{
	if(sa->sa_family == AF_INET) {
		return ntohs(((struct sockaddr_in*)sa)->sin_port);
	}
}
/*****************************************************************/

/***********************create functions***************************/
/*create a socket for proxy to listen*/
/*
 * return 0 for success, 2 for error
 */
int create_proxy() {
	/*getaddrinfo*/
	struct addrinfo hints, *servinfo, *res;
	struct sockaddr res_addr;
	//struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	int rv;

	/*set hints for getaddrinfo()*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	/*getaddrinfo is used to get crucial info for sock() and bind()*/
	if((rv = getaddrinfo(NULL, "0", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		exit(1);
	}

	/*loop all result and try to bind one until succeed*/
	for(res = servinfo; res != NULL; res = res->ai_next) {
		proxy_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(proxy_sockfd == -1) {
			perror("proxy:socket");
			continue;
		}

		if(bind(proxy_sockfd, res->ai_addr, res->ai_addrlen) == -1) {
			/*cannot bind this socket*/
			close(proxy_sockfd);
			perror("proxy: bind");
			continue;
		}
		/*getsockname after successfully bind socket*/
		addrlen = (socklen_t)sizeof res_addr;
		if((getsockname(proxy_sockfd, &res_addr, &addrlen)) == -1) {
			close(proxy_sockfd);
			perror("proxy:getsockname");
			continue;
		}
		//res_out_addr = (struct sockaddr_in*)&res_addr;
		//proxy_port = ntohs(res_out_addr->sin_port);
		proxy_port = get_port(&res_addr);
		break;
	}
	/*release mem*/
	freeaddrinfo(servinfo);
	/*bind failed*/
	if (res == NULL) {
		fprintf(stderr, "proxy:failed to bind socket\n");
		return 2;
	}
	return 0;
}
/*******************create router*****************/
/*
 * return 0 for success, 2 for error
 */
int create_router(int num) {
	struct addrinfo hints, *routinfo, *res;
	struct sockaddr res_addr;
	//struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	int rv;

	/*set hints for getaddrinfo()*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if((rv = getaddrinfo(NULL, "0", &hints, &routinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		return 1;
	}
	/* loop through all the results and make socket*/
	for(res = routinfo; res !=NULL; res = res->ai_next) {
		if ((router_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
			perror("router: socket");
			continue;
		}
		if(bind(router_sockfd, res->ai_addr, res->ai_addrlen) == -1) {
			/*cannot bind this socket*/
			close(router_sockfd);
			perror("router: bind");
			continue;
		}
		/*getsockname after successfully bind socket*/
		addrlen = (socklen_t)sizeof res_addr;
		if((getsockname(router_sockfd, &res_addr, &addrlen)) == -1) {
			close(router_sockfd);
			perror("router:getsockname");
			continue;
		}
		//res_out_addr = (struct sockaddr_in*)&res_addr;
		//router_port = ntohs(res_out_addr->sin_port);
		router_port = get_port(&res_addr);
		break;
	}
	/*release mem*/
	freeaddrinfo(routinfo);
	/*bind failed*/
	if (res == NULL) {
		fprintf(stderr, "router:failed to bind socket\n");
		return 2;
	}
	return 0;
}
/********************************************************************/

/*********************communication functions************************/
/*proxy UDP reader*/
int proxy_udp_reader(char *buffer, int count) {
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	//char buf[MAXBUFLEN];
	char s[INET6_ADDRSTRLEN];

	printf("\nproxy: waiting to recvfrom....\n");

	addr_len = sizeof their_addr;

	numbytes = recvfrom(proxy_sockfd, buffer, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len);

	if(numbytes != -1) {
		printf("proxy: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s));
		printf("proxy: packet is %d bytes long\n", numbytes);
		buffer[numbytes] = '\0';
		/*get information of router*/
		rec_router_port[count] = get_port((struct sockaddr *)&their_addr);
	}

	if (numbytes == -1) {
		perror("recvfrom");
		exit(1);
	}
	return 0;
}
/*router UDP reader*/
int router_udp_reader(char *buffer) {
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	printf("\nrouter: waiting to recvfrom....\n");

	addr_len = sizeof their_addr;

	numbytes = recvfrom(router_sockfd, buffer, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len);
	
	if(numbytes != -1) {
		printf("router: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s));
		printf("router: packet is %d bytes long\n", numbytes);
		buffer[numbytes] = '\0';
	}
	if(numbytes == -1) {
		perror("recvform");
		exit(1);
	}
	return 0;
}
/*router UDP sender*/
int router_udp_sender(char *sendmsg) {
	struct addrinfo hints, *servinfo, *res;
	struct sockaddr res_addr;
	//struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	int sendsocket;
	int numbytesent;
	int rv;
	/*change int portnum to char*/
	char proxyport[PORTLEN];
	sprintf(proxyport,"%d",proxy_port);

	/*set hints for getaddrinfo()*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	/*get proxy info*/
	if((rv = getaddrinfo(NULL, proxyport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		return 1;
	}
	/*create send socket*/
	for (res = servinfo; res != NULL; res = res->ai_next) {
		sendsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sendsocket == -1) {
			perror("router:sendsocket");
		}
	}
	/*send infomation*/
	numbytesent = sendto(sendsocket, sendmsg, strlen(sendmsg), 0, res->ai_addr, res->ai_addrlen);
	if (numbytesent == -1) {
		perror("router:sendto");
		exit(1);
	}
	freeaddrinfo(servinfo);
	return 0;
}
/*proxy UDP sender*/
int proxy_udp_sender(int num, char *sendmsg) {
	struct addrinfo hints, *routinfo, *res;
	struct sockaddr res_addr;
	//struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	int sendsocket;
	int numbytesent;
	int rv;
	/*change int portnum to char*/
	char routport[PORTLEN];
	sprintf(routport,"%d",rec_router_port[num]);

	/*set hints for getaddrinfo()*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	/*get router info*/
	if((rv = getaddrinfo(NULL, routport, &hints, &routinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		return 1;
	}
	/*create sendsocket*/
	for (res = routinfo; res != NULL; res = res->ai_next) {
		sendsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sendsocket == -1) {
			perror("proxy:sendsocket");
		}
	}
	/*send infomation*/
	numbytesent = sendto(sendsocket, sendmsg, strlen(sendmsg), 0, res->ai_addr, res->ai_addrlen);
	if (numbytesent == -1) {
		perror("proxy:sendto");
		exit(1);
	}
	freeaddrinfo(routinfo);
	return 0;
}
/********************************************************************/
