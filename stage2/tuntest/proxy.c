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

#define PORTNUM "0"

#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int tun_alloc(char *dev, int flags) 
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = (char*)"/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR)) < 0 ) 
    {
	perror("Opening /dev/net/tun");
	return fd;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) 
    {
	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) 
    {
	perror("ioctl(TUNSETIFF)");
	close(fd);
	return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}
int tunnel_reader(char *buffer)
{
    char tun_name[IFNAMSIZ];
    //char buffer[2048];

    /* Connect to the tunnel interface (make sure you create the tunnel interface first) */
    strcpy(tun_name, "tun1");
    int tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI); 

    if(tun_fd < 0)
    {
	perror("Open tunnel interface");
	exit(1);
    }


    /*
     * This loop reads packets from the tunnle interface.
     *
     * You will need to REWRITE this loop into a select loop,
     * so that it can talk both to the tun interface,
     * AND to the router.
     *
     * You will also probably want to do other setup in the
     * main() routine.
     */
   // while(1) 
   // {
   // /* Now read data coming from the tunnel */
   //     int nread = read(tun_fd,buffer,sizeof(buffer));
   // if(nread < 0) 
   // {
   //     perror("Reading from tunnel interface");
   //     close(tun_fd);
   //     exit(1);
   // }
   // else
   // {
   //     printf("Read a packet from tunnel, packet length:%d\n", nread);
   // 	printf("packet content: %s\n", buffer);
   //     /* Do whatever with the data, function to manipulate the data here */

   //     /*
   //      * For project A, you will need to add code to forward received packet 
   //      * to router via UDP socket.
   //      * And when you get icmp echo reply packet from router, you need to write
   //      * it back to the tunnel interface
   //      */
   //     
   // }
   // }
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
			perror("listener:socket");
			continue;
		}

		if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
			/*cannot bind this socket*/
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		/*getsockname after successfully bind socket*/
		addrlen = (socklen_t)sizeof res_addr;
		if((getsockname(sockfd, &res_addr, &addrlen)) == -1) {
			close(sockfd);
			perror("listener:getsockname");
			continue;
		}
		res_out_addr = (struct sockaddr_in*)&res_addr;
		newPort = ntohs(res_out_addr->sin_port);
		printf("New assigned port number:%d\n", newPort);
		break;
	}
	if (res == NULL) {
		fprintf(stderr, "listener:failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);
	/*waiting for recvfrom*/
	printf("listener: waiting to recvfrom....");

	addr_len = sizeof their_addr;
	while(1){
		numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len);
		if (numbytes == -1) {
			perror("recvfrom");
			exit(1);
		}
		printf("listener: got packet from %s\n",
				inet_ntop(their_addr.ss_family,
					get_in_addr((struct sockaddr *)&their_addr),
					s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
	}
    close(sockfd);
	return 0;
}
