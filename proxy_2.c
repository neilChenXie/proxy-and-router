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
#include "func.h"

#define PORTNUM "0"
#define PORTLEN 10
#define MAXBUFLEN 100
#define LINELEN 50
#define FNAMELEN 20

int main(int argc, char *argv[])
{
	/*check the input*/
	if (argc != 2) {
		fprintf(stderr,"usage:second arguement is file name\n");
		return -1;
	}
	FILE *fp=NULL, *proxyfp=NULL, *routfp=NULL;
	char line[LINELEN];
	int stage = 0;
	int numrout = 0;
	int rv;
	char filename[FNAMELEN];
	char recline[LINELEN];
	
	/*getaddrinfo*/
	//int rv;
	struct addrinfo hints, *servinfo, *res;
	/*multi routers*/
	//int numrout = atoi(argv[1]);
	int count = 0;
	/*socket*/
	int sockfd;
	struct sockaddr res_addr;
	struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	unsigned short newPort;
	/*fork process*/
	int fpid;
	/*recfrom*/
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char buf[MAXBUFLEN];
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	/*read config file*/
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
	}
	while(fgets(line, sizeof(line), fp) != NULL &&(stage ==0 || numrout == 0)) {
		if (stage == 0) {
			rv = stage_line(line);
			if(rv > 0) {
				stage = rv;
			}
			continue;
		} else {
			/*num of routers*/
			rv = router_line(line);
			if(rv > 0) {
				numrout = rv;
			}
			continue;
		}
	}
	printf("stage %d\n", stage);
	printf("num_routers %d\n", numrout);
	fclose(fp);
	/*getaddrinfo is used to get crucial info for sock() and bind()*/
	if((rv = getaddrinfo(NULL, PORTNUM, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		exit(1);
	}
	/*loop all result and try to bind one until succeed*/
	for(res = servinfo; res != NULL; res = res->ai_next) {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd == -1) {
			perror("proxy:socket");
			continue;
		}

		if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
			/*cannot bind this socket*/
			close(sockfd);
			perror("proxy: bind");
			continue;
		}
		/*getsockname after successfully bind socket*/
		addrlen = (socklen_t)sizeof res_addr;
		if((getsockname(sockfd, &res_addr, &addrlen)) == -1) {
			close(sockfd);
			perror("proxy:getsockname");
			continue;
		}
		res_out_addr = (struct sockaddr_in*)&res_addr;
		newPort = ntohs(res_out_addr->sin_port);
		//sprintf(recline, "proxy port:%d\n", newPort);
		//fputs(recline, proxyfp);
		break;
	}
	if (res == NULL) {
		fprintf(stderr, "proxy:failed to bind socket\n");
		return 2;
	}
	/********************************/

	/*create proxy log*/
	strcpy(filename, "stage1.proxy.out");
	if((proxyfp = fopen(filename, "w+"))==NULL) {
		fprintf(stderr, "Cannot open/create proxy log file\n");
		exit(1);
	} else {
		if(fseek(proxyfp, 0L, SEEK_END) == -1) {
			printf("fseek doesn\'t work");
		}
		sprintf(recline,"stage1\nproxy port:%d\n",newPort);
		fputs(recline,proxyfp);
		fclose(proxyfp);
	}
	/***********************************/

	freeaddrinfo(servinfo);
	/*fork router process*/
	while(count < numrout) {
		fpid = fork();
		if(fpid != 0) {
			/*waiting for recvfrom*/
			printf("\nproxy: waiting to recvfrom....\n");

			addr_len = sizeof their_addr;
			if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) != -1) {
				printf("proxy: got packet from %s\n",
						inet_ntop(their_addr.ss_family,
							get_in_addr((struct sockaddr *)&their_addr),
							s, sizeof s));
				printf("proxy: packet is %d bytes long\n", numbytes);
				buf[numbytes] = '\0';
				printf("proxy: packet contains \"%s\"\n", buf);
				printf("router: %d, pid %s, port: %d\n", count+1, buf,get_port((struct sockaddr *)&their_addr));
				/*record*/
				strcpy(filename, "stage1.proxy.out");
				if((proxyfp = fopen(filename, "r+"))==NULL) {
					fprintf(stderr, "Cannot open/create proxy log file\n");
					exit(1);
				} else {
					if(fseek(proxyfp, 0L, SEEK_END) == -1) {
						printf("fseek doesn\'t work");
					}
					sprintf(recline, "router: %d, pid %s, port: %d\n", count+1, buf,get_port((struct sockaddr *)&their_addr));
					fputs(recline, proxyfp);
					fclose(proxyfp);
				}
			}
			if (numbytes == -1) {
				perror("recvfrom");
				exit(1);
			}
		}
		if(fpid == 0) {
			/*child process*/
			int pid = getpid();
			struct addrinfo *routinfo;
			char myPort[PORTLEN];
			int mySock;
			char sendmsg[LINELEN];
			int numbytesent;
			sprintf(myPort, "%d", newPort);
			printf("router:I,m child process: %d\n", pid);
			sprintf(sendmsg,"%d",pid);

			if((rv = getaddrinfo(NULL, myPort, &hints, &routinfo)) != 0) {
				fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
				return 1;
			}
			/* loop through all the results and make socket*/
			for(res = routinfo; res !=NULL; res = res->ai_next) {
				if ((mySock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
					perror("router: socket");
					continue;
				}
				break;
			}
			if (res == NULL) {
				fprintf(stderr, "router:failed to bind socket\n");
				return 2;
			}
			/*send infomation & get port num*/
			numbytesent = sendto(mySock, sendmsg, strlen(sendmsg), 0, res->ai_addr, res->ai_addrlen);
			if (numbytesent == -1) {
				perror("router:sendto");
				exit(1);
			}
			freeaddrinfo(routinfo);

			printf("router: sent %d bytes\n", numbytesent);
			/*print own port num*/
			addrlen = (socklen_t)sizeof res_addr;
			if((getsockname(mySock, &res_addr, &addrlen)) == -1) {
				close(sockfd);
				perror("proxy:getsockname");
				continue;
			}
			res_out_addr = (struct sockaddr_in*)&res_addr;
			newPort = ntohs(res_out_addr->sin_port);
			printf("router: router assigned port number:%d\n", newPort);
			/*recorde*/
			/*routers*/
			sprintf(filename, "stage1.router%d.out",count+1);
			if((routfp = fopen(filename, "w+"))==NULL) {
				fprintf(stderr, "Cannot open/create proxy log file\n");
				exit(1);
			}
			/*move to end of the file*/
			fseek(routfp, 0, SEEK_END);
			sprintf(recline,"router: %d, pid: %d, port: %d\n", count+1, pid, newPort);
			fputs(recline,routfp);
			fclose(routfp);

			exit(0);
		}
		count++;
	}
	return 0;
}
