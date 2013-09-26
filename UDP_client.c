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
#include <time.h>

#define SERVERPORT "4950"	// the port users will be connecting to
#define INDEXLEN 4
#define PACKETLEN 5000

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

/////////////////////////////////////////////////////////////
	char *fname = argv[2];
	FILE *input = fopen(fname, "rb");
	// char index[INDEXLEN];

	char data[INDEXLEN + PACKETLEN];
	unsigned int i=0;
	int readnum;
	int lastsize = PACKETLEN;
	int test;
	while (!feof(input))
	{
		readnum = fread(data, sizeof(char), PACKETLEN, input);
		// printf("readnum\n");
		data[PACKETLEN + 0] = i >> 24;
		data[PACKETLEN + 1] = i >> 16;
		data[PACKETLEN + 2] = i >> 8;
		data[PACKETLEN + 3] = i;
		// printf("%s\n", data);
		if (readnum != PACKETLEN)
		{	
			lastsize = readnum;
			printf("%d, %d\n", lastsize, readnum);
		}
		
		do{
			sleep(0.05);
			test = sendto(sockfd, data, INDEXLEN + PACKETLEN, 0, p->ai_addr, p->ai_addrlen);
		}
		while(test<( INDEXLEN + PACKETLEN));

		printf("%d\n", test);
		printf("%s\n", data);
		i++;
	}
	char ones = 255;
	data[PACKETLEN + 0] = ones;
	data[PACKETLEN + 1] = ones;
	data[PACKETLEN + 2] = lastsize >> 8;
	data[PACKETLEN + 3] = lastsize;
	printf("%d\n", lastsize);
	// printf("%d, %d\n", data[PACKETLEN], data[PACKETLEN+1]);
	// printf("%c,%c,%c,%c\n", data[0],data[1],data[2],data[3]);
	sendto(sockfd, data, INDEXLEN + PACKETLEN, 0, p->ai_addr, p->ai_addrlen);
	// sendto(sockfd, &count, 1, 0, p->ai_addr, p->ai_addrlen);
/////////////////////////////////////////////////////////////
	freeaddrinfo(servinfo);

	close(sockfd);

	return 0;
}