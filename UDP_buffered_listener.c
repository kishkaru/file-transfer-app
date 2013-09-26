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
#include <fcntl.h>
#include <time.h>

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

typedef struct node {
	char *ptr;
	struct node *next;
} node;

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	p = servinfo;
	sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	bind(sockfd, p->ai_addr, p->ai_addrlen);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recv...\n");

	int i;
	node nodes[10]; //enough to run our tests
	for(i=0; i < 10; i++) {
		nodes[i].next = NULL;
	}

	int j = 0;
	int k = 0;
	int count = 0;

	time_t currTime;
	time_t pastTime;
	time(&currTime);
	time(&pastTime);

	addr_len = sizeof their_addr;
	while(count < 10){ //while 10 packets printed on screen
		numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len); 
		time(&currTime); //get the current time
		
		//if packet recieved, add it to link list
		if(numbytes != -1){
			j++;
			nodes[j-1].next = &nodes[j];
		}

		//calculate time diff
		double time_diff = difftime(currTime,pastTime);

		//if its been 5 secs since last printing...
		if(time_diff >= 5){
			time(&pastTime);
			if(nodes[k].next != NULL){ //and there is a packet available...
				printf("%s\n", "Got a packet!!"); //print it
				k++;
				count++;
			}
		}
	}

	close(sockfd);
	return 0;
}
