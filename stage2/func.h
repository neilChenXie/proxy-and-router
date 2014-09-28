#define PORTNUM "0"
#define PORTLEN 10
#define MAXBUFLEN 100
#define LINELEN 50
#define FNAMELEN 20
#define MAXROUTER 10
#define MAXMSGLEN 100
extern int proxy_sockfd;
extern int proxy_port;
extern int router_sockfd;
extern int router_port;
extern int rec_router_port[MAXROUTER];
extern int num_stage;
extern int num_router;

int stage_line(char *sp);
int router_line(char *sp);
int read_config(FILE *fp);
int write_file(char *filename, char *cont);
void *get_in_addr(struct sockaddr *sa);
unsigned short get_port(struct sockaddr *sa);
int create_proxy(); 
int create_router();
int proxy_udp_reader(char *buffer, int count);
int router_udp_reader(char *buffer);
int router_udp_sender(char *sendmsg);
int proxy_udp_sender(int num, char *sendmsg);
