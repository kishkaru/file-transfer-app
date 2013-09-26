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

#define MYPORT "4950"	// the port users will be connecting to

#define PACKETLEN 5000
#define INDEXLEN 4
#define CONTENTLEN 100000

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char *buf;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int packet_amount;
	// struct data_node **table;
	int i;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

//////////////////////////////////////////////////////////////
	// printf("1\n");
	addr_len = sizeof their_addr;
	char *content[CONTENTLEN];
	for (i=0; i < CONTENTLEN; i++)
	{
		content[i] = malloc(sizeof(char) * PACKETLEN);
		memset(content[i], 0, sizeof(char) * PACKETLEN);
	}
	memset(content, 0, sizeof(char*));
	unsigned int index=0;
	int readnum;
	int lastsize;
	char ones = 255;
	// printf("2\n");
	int test;
	buf = malloc(sizeof(char) * (PACKETLEN + INDEXLEN));
	// test = recvfrom(sockfd, buf, INDEXLEN, 0, (struct sockaddr *)&their_addr, &addr_len);
	// printf("%d\n", test);
	int count=0;
	while (recvfrom(sockfd, buf, INDEXLEN + PACKETLEN, 0, (struct sockaddr *)&their_addr, &addr_len))
	{
		// printf("3\n");
		// printf("%d, %d\n", buf[PACKETLEN + 0], buf[PACKETLEN + 1]);
		if ((buf[PACKETLEN + 0] == ones) && (buf[PACKETLEN + 1] == ones))
		{
			// lastsize = buf[PACKETLEN + 2];
			// lastsize = lastsize << 8 + buf[PACKETLEN + 3];
			lastsize = (unsigned char)buf[PACKETLEN + 2] * 256 + (unsigned char)buf[PACKETLEN + 3];
			goto done;
		}

		index = (unsigned char)buf[PACKETLEN + 0] * 16777216 + (unsigned char)buf[PACKETLEN + 1] * 65536 + (unsigned char)buf[PACKETLEN + 2] * 256 + (unsigned char)buf[PACKETLEN + 3];

		// index = buf[PACKETLEN + 0];
		// printf("%d\n", buf[PACKETLEN + 0]);
		// printf("%d!\n", index);
		// index = (index << 8) + buf[PACKETLEN + 1];
		// printf("%d\n", buf[PACKETLEN + 1]);
		// printf("%d!\n", index);
		// index = (index << 8) + buf[PACKETLEN + 2];
		// printf("%d\n", buf[PACKETLEN + 2]);
		// printf("%d!\n", index);
		// index = (index << 8) + buf[PACKETLEN + 3];
		// printf("%d\n", buf[PACKETLEN + 3]);
		// printf("%d!\n", index);
		content[index] = buf;
		buf = malloc(sizeof(char) * (PACKETLEN + INDEXLEN));
		count++;
	}
	done:
	// printf("%d\n", count);
	printf("done receiving\n");
	char fname[] = "out.txt";
	FILE *output = fopen(fname, "wb");
	for (i=0; i < count; i++)
	{
		// if (content[i] == 0)
		if (i == count - 1)
		{
			fwrite(content[i], sizeof(char), lastsize, output);
			printf("!!\n");
			printf("%d\n", lastsize);
			goto out;
		}
		else
			fwrite(content[i], sizeof(char), PACKETLEN, output);
	}
	out:
	// char end = EOF;
	// fwrite(&end, sizeof(char), 1, output);
	printf("Done.\n");
	fclose(output);

	// char msg[] = "Received.";
	// if (fopen(fname, "rb"))
	// 	send(server_socket, msg, sizeof(msg)-1, 0);
	// else printf("Receive error\n");

//////////////////////////////////////////////////////////////

	close(sockfd);

	return 0;
}