#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int listen_on_socket, client_socket;  // listen on sock_fd, new connection on client_socket
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	if (argc != 2) {
		fprintf(stderr,"usage: server filename\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((listen_on_socket = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (bind(listen_on_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(listen_on_socket);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(listen_on_socket, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		client_socket = accept(listen_on_socket, (struct sockaddr *)&their_addr, &sin_size);
		if (client_socket == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(listen_on_socket); // child doesn't need the listener
			
/////////////////////////////////////////////////////////////

			char *fname = argv[2];
			FILE *input = fopen(fname, "rb");
			char a;
			while(fread(&a, sizeof(char), 1, input) && !feof(input)) {
				send(client_socket, &a, sizeof(a),0);
			}

			a = -1;
			send(client_socket, &a, sizeof(a), 0);
			fclose(input);
			char ackn[100];
			int numbytes;
			if ((numbytes = recv(client_socket, ackn, sizeof(ackn)-1, 0)) == -1) {
				perror("receiving error.");
			}
			ackn[numbytes] = '\0';
			printf("%s\n", ackn);
			

/////////////////////////////////////////////////////////////
			close(client_socket);
			exit(0);
		}
		close(client_socket);  // parent doesn't need this
	}

	return 0;
}
