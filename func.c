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

#define LINELEN 50

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

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

unsigned short get_port(struct sockaddr *sa) 
{
	if(sa->sa_family == AF_INET) {
		return ntohs(((struct sockaddr_in*)sa)->sin_port);
	}
}

//void printtime()
//{
//	struct timeval recor_out;
//	gettimeofday(&recor_out,NULL);
//	int time_passed = (recor_out.tv_sec-emu_start.tv_sec)*1000000+(recor_out.tv_usec-emu_start.tv_usec);
//	float out_time = time_passed/1000;
//	// printf("%.3fms: ",out_time);Var(X) = E(X2) - [E(X)]2
//	int out_bit[11];
//	int to_recor;
//	int to_divide=1;
//	int i = 0;
//	for(i = 0; i<11; i++)
//	{
//		to_recor = time_passed/to_divide;
//		out_bit[i]=to_recor%10;
//		to_divide = to_divide*10;
//	}
//	printf("%d%d%d%d%d%d%d%d.%d%d%dms: ",out_bit[10],out_bit[9],out_bit[8],out_bit[7],out_bit[6],out_bit[5],out_bit[4],out_bit[3],out_bit[2],out_bit[1],out_bit[0]);
//}
