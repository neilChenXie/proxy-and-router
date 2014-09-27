#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

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
int main (int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr,"usage:second arguement is file name\n");
	}
	FILE *fp=NULL;
	char line[LINELEN];
	int stage = 0;
	int numrout = 0;
	int rv;
	
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
	return 0;
}

