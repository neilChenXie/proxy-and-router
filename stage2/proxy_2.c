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

int num_stage = 0;
int num_router = 0;

int main(int argc, char *argv[])
{
	FILE *fp=NULL, *proxyfp=NULL, *routfp=NULL;
	char line[LINELEN];
	char recline[LINELEN];
	int rv;
	int count = 0;//multi routers
	int fpid;//fork process

	/*check the input*/
	if (argc != 2) {
		fprintf(stderr,"usage:second arguement is file name\n");
		return -1;
	}

	/*read config file*/
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
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
	printf("stage %d\n", num_stage);
	printf("num_router %d\n", num_router);
	fclose(fp);
	/******************/

	/*create proxy log*/
	//strcpy(filename, "stage1.proxy.out");
	//if((proxyfp = fopen(filename, "w+"))==NULL) {
	//	fprintf(stderr, "Cannot open/create proxy log file\n");
	//	exit(1);
	//} else {
	//	if(fseek(proxyfp, 0L, SEEK_END) == -1) {
	//		printf("fseek doesn\'t work");
	//	}
	//	sprintf(recline,"stage1\nproxy port:%d\n",newPort);
	//	fputs(recline,proxyfp);
	//	fclose(proxyfp);
	//}
	/***********************************/

	/*fork router process*/
	while(count < num_router) {
		fpid = fork();
		if(fpid != 0) {
			/*waiting for recvfrom*/
			/*record*/
			//strcpy(filename, "stage1.proxy.out");
			//if((proxyfp = fopen(filename, "r+"))==NULL) {
			//	fprintf(stderr, "Cannot open/create proxy log file\n");
			//	exit(1);
			//} else {
			//	if(fseek(proxyfp, 0L, SEEK_END) == -1) {
			//		printf("fseek doesn\'t work");
			//	}
			//	routport[count] = get_port((struct sockaddr *)&their_addr);
			//	sprintf(recline, "router: %d, pid %s, port: %d\n", count+1, buf,routport[count]);
			//	fputs(recline, proxyfp);
			//	fclose(proxyfp);
			//}
			/**********/
		}
		if(fpid == 0) {
			/*child process*/
			int pid = getpid();
			printf("router:I,m child process: %d\n", pid);
			printf("%d",pid);

			/*recorde*/
			/*routers*/
			//sprintf(filename, "stage1.router%d.out",count+1);
			//if((routfp = fopen(filename, "w+"))==NULL) {
			//	fprintf(stderr, "Cannot open/create proxy log file\n");
			//	exit(1);
			//}
			///*move to end of the file*/
			//fseek(routfp, 0, SEEK_END);
			//sprintf(recline,"router: %d, pid: %d, port: %d\n", count+1, pid, newPort);
			//fputs(recline,routfp);
			//fclose(routfp);
			/**************************/

			exit(0);
		}
		count++;
	}
	return 0;
}
