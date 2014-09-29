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
#include <net/if.h>
#include <fcntl.h>
#include "func.h"


int main(int argc, char *argv[])
{
	FILE *fp=NULL, *proxyfp=NULL, *routfp=NULL;
	char recline[LINELEN];
	char filename[FNAMELEN];
	int rv;
	int count;//multi routers
	int fpid;//fork process

	/*check the input*/
	if (argc != 2) {
		fprintf(stderr,"usage:second arguement is file name\n");
		return -1;
	}
	/*****************/

	/*read config file*/
	fp = fopen(argv[1], "r");
	if(fp == NULL) {
		fprintf(stderr,"Cannot open file: %s",argv[1]);
		return -1;
	}
	read_config(fp);
	fclose(fp);
	printf("stage: %d\n",num_stage);
	printf("router: %d\n",num_router);
	/******************/

	/*create proxy*/
	if(create_proxy() != 0) {
		fprintf(stderr,"Cannot create proxy\n");
		return -1;
	}
	printf("proxy_port:%d\n", proxy_port);
	printf("proxy_socket:%d\n",proxy_sockfd);
	/**************/

	/*create proxy log*/
	sprintf(filename, "stage%d.proxy.out",num_stage);
	if((proxyfp = fopen(filename, "w+"))==NULL) {
		fprintf(stderr, "Cannot create proxy log file\n");
		exit(1);
	} else {
	//	if(fseek(proxyfp, 0L, SEEK_END) == -1) { //go to end of log file
	//		printf("fseek doesn\'t work");
	//	}
		sprintf(recline,"stage1\nproxy port:%d\n",proxy_port);
		fputs(recline,proxyfp);
		fclose(proxyfp);
	}
	/***********************************/

	/*fork router process*/
	count = 0;
	while(count < num_router) {
		fpid = fork();
		if(fpid != 0) {
			/*waiting for recvfrom*/
			char buffer[MAXBUFLEN];
			if(proxy_udp_reader(buffer, count) != 0) {
				fprintf(stderr, "Cannot get packets from router\n");
			}
			printf("proxy receive:%s\n", buffer);
			/*record*/
			sprintf(filename, "stage%d.proxy.out", num_stage);
			sprintf(recline, "router: %d, pid %s, port: %d\n", count+1, buffer, rec_router_port[count]);
			if(write_file(filename, recline) != 0) {
				fprintf(stderr, "Cannot write to file: %s\n", filename);
			}
			/**********/
		}
		if(fpid == 0) {
			/*router subroutine*/
			// the count var can let router to know the num of itself
			int pid = getpid();
			char sendmsg[MAXMSGLEN];

			printf("router:I,m child process: %d\n", pid);
			
			/*create router socket*/
			create_router();
			printf("router_port:%d\n", router_port);
			printf("router_socket:%d\n", router_sockfd);

			/*send pid to proxy*/
			sprintf(sendmsg,"%d", pid);
			router_udp_sender(sendmsg);
			/*recorde*/
			/*create router log file*/
			sprintf(filename, "stage%d.router%d.out", num_stage,count+1);
			if((routfp = fopen(filename, "w+"))==NULL) {
				fprintf(stderr, "Cannot open/create proxy log file\n");
				exit(1);
			}
			sprintf(recline,"router: %d, pid: %d, port: %d\n", count+1, pid, router_port);
			fputs(recline,routfp);
			fclose(routfp);
			/**************************/
			/*for stage 2 of router*/
			/***********************/
			exit(0);
		}
		count++;
	}
	/*for stage 2 of proxy*/
	char stage2buf[MAXBUFLEN];
	if(tunnel_create() != 0) {
		fprintf(stderr, "proxy:cannot connect to tunnel");
		exit(1);
	}
	while(1) {
		tunnel_reader(stage2buf);
		sprintf(filename, "stage%d.proxy.out",num_stage);
		if(write_file(filename, stage2buf) != 0) {
			fprintf(stderr, "proxy: cannot write to file");
			exit(1);
		}
	}
	/**********************/
	return 0;
}
