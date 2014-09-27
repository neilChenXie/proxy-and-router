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

#define PORTNUM "0"

#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	/*getaddrinfo*/
	int rv;
	struct addrinfo hints, *servinfo, *res;
	/*socket*/
	int sockfd;
	struct sockaddr res_addr;
	struct sockaddr_in *res_out_addr;
	socklen_t addrlen;
	unsigned short newPort;
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
	/*getaddrinfo is used to get crucial info for sock() and bind()*/
	if((rv = getaddrinfo(NULL, PORTNUM, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(rv));
		return 1;
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
		printf("New assigned port number:%d\n", newPort);
		break;
	}
	if (res == NULL) {
		fprintf(stderr, "proxy:failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);
	/*waiting for recvfrom*/
	printf("proxy: waiting to recvfrom....");

	addr_len = sizeof their_addr;
	while((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) != -1) {
		printf("proxy: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s));
		printf("proxy: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("proxy: packet contains \"%s\"\n", buf);
	}
	if (numbytes == -1) {
		perror("recvfrom");
		exit(1);
	}
    close(sockfd);
	return 0;
}
