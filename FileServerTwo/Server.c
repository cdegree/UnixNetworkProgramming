/*
 * Server.c
 * 多线程客户端文件传输服务器
 *  Created on: 2014年12月26日
 *      Author: 陈学位
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>//bzero
#include <time.h>

struct ClientInfo {
	int ssock;
	struct sockaddr_in fsin;
};

int CommunicateWork(void *c);
int passiveTCP(const char *service, int qlen);
int passivesock(const char *service, const char *transport, int qlen);
#define FILE_NAME_SIZE	512
#define BUFFER_SIZE 1024
#define QLEN 1024
unsigned short portbase = 0; /* port base, for non-root servers	*/

/*------------------------------------------------------------------------
 * main - Iterative TCP server for service
 *------------------------------------------------------------------------
 */
int main(int argc, char **argv) {

	pthread_t id = NULL;
	struct sockaddr_in fsin; /* the from address of a client	*/
	char *service = "PORT"; /* service name or port number	*/
	int msock, ssock; /* master & slave sockets	*/
	unsigned int alen; /* from-address length		*/
	switch (argc) {
	case 1:
		break;
	case 2:
		service = argv[1];
		break;
	default:
		printf("usage: Server [port]\n");
	}
	msock = passiveTCP(service, QLEN);
	struct ClientInfo *newClient = (struct ClientInfo*) malloc(
			sizeof(struct ClientInfo));

	while (1) {
		id = 1;
		alen = sizeof(fsin);
		ssock = accept(msock, (struct sockaddr *) &fsin, &alen);

		printf("Accepted a client from address = %s   port = %d\n",
				inet_ntoa(fsin.sin_addr), ntohs(fsin.sin_port));
		newClient->ssock = ssock;
		newClient->fsin = fsin;

		//printf("Accepted a client from address = %s   port = %d\n",inet_ntoa( newClient->fsin.sin_addr),ntohs(newClient->fsin.sin_port));
		if (ssock < 0)
			printf("accept failed\n");
		//CommunicateWork(ssock);
		if (pthread_create(&id, NULL, (void * (*)(void *)) CommunicateWork,
				(void *) newClient) < 0) {
			puts("create thread failed");
		}
	}
	pthread_join(id, NULL);
	close(msock);
}
/*------------------------------------------------------------------------
 * CommunicateWork - do TCP protocol
 *
 *------------------------------------------------------------------------
 */
int CommunicateWork(void *c) {
	int n, i;
	char buf[BUFFER_SIZE];
	int fd = ((struct ClientInfo*) c)->ssock;
	struct sockaddr_in cfsin = ((struct ClientInfo*) c)->fsin;
	//printf("Accepted a client from address = %s   port = %d\n",inet_ntoa(cfsin.sin_addr),ntohs(cfsin.sin_port));
	char file_name[FILE_NAME_SIZE];
	while ((n = read(fd, file_name, FILE_NAME_SIZE)) > 0) {
		file_name[n] = '\0'; /* ensure null-terminated	*/
		printf(
				"from %s.%d received a download request  for file:%s\n",
				inet_ntoa(cfsin.sin_addr), ntohs(cfsin.sin_port), file_name);
		FILE *file = fopen(file_name, "r");
		if (NULL == file) {
			printf("File:%s NOT FOUND\n", file_name);
			char *msg = "FILE NOT FOUND";
			strcpy(buf,msg);
			//向客户端反馈文件为找到
			if (send(fd, buf,strlen(buf), 0) < 0) {
				perror("NOT FOUND Message Send Failed");
			}
			fflush(stdout);
			continue;
		}
		bzero(buf, BUFFER_SIZE);
		int length = 0;
		//每从文件中读取字符，就传送
		while ((length = fread(buf, sizeof(char), BUFFER_SIZE, file)) > 0) {
			if (send(fd, buf, length, 0) < 0) {
				printf("File:%s Send Failed\n", file_name);
				break;
			}
			send(fd,'\0',1,0);
			bzero(buf, BUFFER_SIZE);
		}
		fclose(file);
		printf("File:%s Send Successful\n", file_name);
		//printf("Received from client message :%s\n",buf);
		fflush(stdout);
		close(fd);
		break;
	}
	printf("Closed");
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
