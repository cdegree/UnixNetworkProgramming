/* TCPdaytimed.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAXDATASIZE	1000

struct Client {
	int fd;
	char *name;
	struct sockaddr_in addr;
	char *data;
};

int CommunicateWork(struct Client* client, char *recvbuf, int n);
int passiveTCP(const char *service, int qlen);
int passivesock(const char *service, const char *transport, int qlen);
#define QLEN	1280
unsigned short portbase = 0; /* port base, for non-root servers	*/

/*------------------------------------------------------------------------
 * main - Iterative TCP server for service
 *------------------------------------------------------------------------
 */
;
int main(int argc, char **argv) {
	char *service = "service"; /* service name or port number	*/
	switch (argc) {
	case 1:
		break;
	case 2:
		service = argv[1];
		break;
	default:
		printf("usage: Server [port]\n");
	}

	int i, maxi, maxfd, sockfd;
	int nready;
	ssize_t n;
	fd_set rset, allset;
	int listenfd, connectfd;
	struct Client client[FD_SETSIZE]; /* the from address of a client	*/
	char recvbuf[MAXDATASIZE];
	socklen_t sin_size;/* from-address length		*/

	/*
	 * listen steps
	 */

	listenfd = passiveTCP(service, QLEN);
	sin_size = sizeof(struct sockaddr_in);
	maxfd = listenfd;
	maxi = -1;

	for (i = 0; i < FD_SETSIZE; i++)
		client[i].fd = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	while (1) {
		//puts("one circle");
		fflush(stdout);
		struct sockaddr_in addr;
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(listenfd, &rset)) {
			connectfd = accept(listenfd, (struct sockaddr *) &addr, &sin_size);
			if (connectfd == -1) {
				printf("accept error\n");
				continue;
			}
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i].fd < 0) {
					client[i].fd = connectfd;
					client[i].name = (char *) malloc(
							sizeof(char) * MAXDATASIZE);
					client[i].addr = addr;
					client[i].data = (char *) malloc(
							sizeof(char) * MAXDATASIZE);
					client[i].name[0] = '\0';
					client[i].data[0] = '\0';
					printf("You got a connect from %s.%d\n",
							inet_ntoa(client[i].addr.sin_addr), ntohs(client[i].addr.sin_port));
					fflush(stdout);
					break;
				}
			if (i == FD_SETSIZE) {
				printf("too many clients.\n");
			}
			FD_SET(connectfd, &allset);
			if (connectfd > maxfd)
				maxfd = connectfd;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0)
				continue;
		} /* if (FD_ISSET (listenfd… */
		for (i = 0; i <= maxi; i++) {
			if ((sockfd = client[i].fd) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ((n = recv(sockfd, recvbuf, MAXDATASIZE, 0)) == 0) {
					close(sockfd);
					printf("Client (%s) closed connection. User’s data: %s\n",
							client[i].name, client[i].data);
					fflush(stdout);
					FD_CLR(sockfd, &allset);
					client[i].fd = -1;
					free(client[i].name);
					free(client[i].data);
				} else {

					CommunicateWork(&client[i], recvbuf, n);
				}
				if (--nready <= 0)
					break;
			} /* if (FD_ISSET(sockfd, &rset))  */
		} /* for(i = 0; i <= maxi; i++)   */
		fflush(stdout);
	} /* while(1); */
	close(listenfd);
}
/*------------------------------------------------------------------------
 * ReceiveWork - do TCP protocol
 *------------------------------------------------------------------------
 */
int CommunicateWork(struct Client *client, char *recvbuf, int n) {
	int fd = client->fd;
	struct sockaddr_in cfsin = client->addr;
	recvbuf[n] = '\0';
	printf("Received message :%s\tfrom client address = %s port = %d\n ", recvbuf,
			inet_ntoa(cfsin.sin_addr), ntohs(cfsin.sin_port));
/*	for (i = 0; i < n / 2; ++i) {
		char tmp = recvbuf[i];
		recvbuf[i] = recvbuf[n - i - 1];
		recvbuf[n - i - 1] = tmp;
	}*/
//printf("Received from client message :%s\n",buf);
	write(fd, recvbuf, n);
	fflush(stdout);

	return 0;
}

/*------------------------------------------------------------------------
 * passiveTCP - create a passive socket for use in a TCP server
 *------------------------------------------------------------------------
 */
int passiveTCP(const char *service, int qlen)
/*
 * Arguments:
 *      service - service associated with the desired port
 *      qlen    - maximum server request queue length
 */
{
	return passivesock(service, "tcp", qlen);
}

/*------------------------------------------------------------------------
 * passivesock - allocate & bind a server socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int passivesock(const char *service, const char *transport, int qlen)
/*
 * Arguments:
 *      service   - service associated with the desired port
 *      transport - transport protocol to use ("tcp" or "udp")
 *      qlen      - maximum server request queue length
 */
{
	printf("****  %s   %s   %d\n", service, transport, qlen);
	struct servent *pse; /* pointer to service information entry	*/
	struct protoent *ppe; /* pointer to protocol information entry*/
	struct sockaddr_in sin; /* an Internet endpoint address		*/
	int s, type; /* socket descriptor and socket type	*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	/* Map service name to port number */
	if ((pse = getservbyname(service, transport))) {
		sin.sin_port = htons(ntohs((unsigned short) pse->s_port) + portbase);

	} else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0) {
		printf("can't get \"%s\" service entry\n", service);
	}
//printf("sin_port = %d\n",sin.sin_port);
	/* Map protocol name to protocol number */
	if ((ppe = getprotobyname(transport)) == 0)
		printf("can't get \"%s\" protocol entry\n", transport);

	/* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	/* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0) {
		printf("can't create socket\n");
	}
//char *ppp = inet_ntoa(sin.sin_addr);
//printf("server host = %s\n",ppp);
	/* Bind the socket */
	if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0)
		printf("can't bind to %s port\n", service);
	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		printf("can't listen on %s port\n", service);
	return s;
}

