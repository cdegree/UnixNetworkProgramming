/* TCPdaytimed.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int 		CommunicateWork(int fd);
int		passiveTCP(const char *service, int qlen);
int		passivesock(const char *service, const char *transport,int qlen);
#define QLEN	128
unsigned short	portbase = 0;	/* port base, for non-root servers	*/
/*------------------------------------------------------------------------
 * main - Iterative TCP server for service
 *------------------------------------------------------------------------
 */
int main(int argc, char **argv) {

	struct	sockaddr_in fsin;	/* the from address of a client	*/
	char	*service = "service";	/* service name or port number	*/
	int	msock, ssock;		/* master & slave sockets	*/
	unsigned int	alen;		/* from-address length		*/
	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		printf("usage: TCPdaytimed [port]\n");
	}
	msock = passiveTCP(service, QLEN);
  
    while (1) {
    		alen = sizeof(fsin);
    		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
    		printf("Accepted a client from address = %s   port = %d\n",inet_ntoa( fsin.sin_addr),ntohs(fsin.sin_port));
    		if (ssock < 0)
    			printf("accept failed\n");
    		CommunicateWork(ssock);

    		(void) close(ssock);
    	}
}
/*------------------------------------------------------------------------
 * ReceiveWork - do TCP protocol
 *------------------------------------------------------------------------
 */
int
CommunicateWork(int fd)
{
	int n,i;
	char buf[QLEN];
	while( (n = read(fd, buf, QLEN)) > 0) {
				buf[n] = '\0';		/* ensure null-terminated	*/
				printf("Received from client message :%s\n",buf);
				n = strlen(buf);
				for(i=0;i<n/2;++i){
					char tmp = buf[i];
					buf[i] = buf[n-i-1];
					buf[n-i-1] = tmp;
				}
				//printf("Received from client message :%s\n",buf);
				write(fd, buf, QLEN);
				fflush(stdout);
	}
	printf("Closed");
	return 0;
}





/*------------------------------------------------------------------------
 * passiveTCP - create a passive socket for use in a TCP server
 *------------------------------------------------------------------------
 */
int
passiveTCP(const char *service, int qlen)
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
int
passivesock(const char *service, const char *transport, int qlen)
/*
 * Arguments:
 *      service   - service associated with the desired port
 *      transport - transport protocol to use ("tcp" or "udp")
 *      qlen      - maximum server request queue length
 */
{
	printf("****  %s   %s   %d\n",service,transport,qlen);
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
	if ( (pse = getservbyname(service, transport) )){
		sin.sin_port = htons(ntohs((unsigned short)pse->s_port)+ portbase);

	}
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0){
		printf("can't get \"%s\" service entry\n", service);
	}
	//printf("sin_port = %d\n",sin.sin_port);
    /* Map protocol name to protocol number */
	if ( (ppe = getprotobyname(transport)) == 0)
		printf("can't get \"%s\" protocol entry\n", transport);

    /* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0){
		printf("can't create socket\n");
	}
	char *ppp = inet_ntoa(sin.sin_addr);
		//printf("server host = %s\n",ppp);
    /* Bind the socket */
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		printf("can't bind to %s port\n",service);
	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		printf("can't listen on %s port\n",service);
	return s;
}


