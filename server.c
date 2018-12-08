#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 5000
#define LISTENQ 100

int open_listenfd(char *port) {
	struct addrinfo hints, *listp, *p;
	int listenfd, optval=1;

	/* get a list of potential server addresses */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;

	getaddrinfo(NULL, port, &hints, &listp);

	/* Walk the list for one that we can bind to */
	for (p = listp; p; p = p->ai_next) {
		/* create socket descriptor */
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			continue;
		}

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));

		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
			break;
		}
		close(listenfd);
	}

	freeaddrinfo(listp);
	if (!p) {
		return -1;
	}
	if (listen(listenfd, LISTENQ) < 0) {
		close(listenfd);
		return -1;
	}
	return listenfd;
}

void processInput(int connfd, float* toUpdate) {
	size_t n;
	char buf[MAXLINE];
	char responsebuf[MAXLINE];

	while((n = read (connfd, buf, MAXLINE)) != 0) {
		printf("server received %d bytes\n", (int)n);

		float float_input = (float) atof(buf);
		*toUpdate = *toUpdate + float_input;

		sprintf(buf, "%.1f\n", *toUpdate);
		write(connfd, buf, strlen(buf));
		//write(connfd, buf, n);
		memset(buf, 0, MAXLINE);
	}
}

int main() {
	float sum = 0.0;

	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	listenfd = open_listenfd("2000");
	while (1) {
		clientlen = sizeof(struct sockaddr_storage);
		connfd = accept(listenfd, (struct sockaddr * restrict) &clientaddr, &clientlen);
		getnameinfo((const struct sockaddr *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", client_hostname, client_port);
		processInput(connfd, &sum);
		close(connfd);
	}
	exit(0);
}
