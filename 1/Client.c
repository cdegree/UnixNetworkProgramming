/* TCPdaytime.c - TCPdaytime, main */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int	TCPdaytime(const char *host, const char *service);
int	connectTCP(const char *host, const char *service);

#define	LINELEN		128

/*------------------------------------------------------------------------
 * main - TCP client for DAYTIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*host = "localhost";	/* host to use if none supplied	*/
	char	*service = "dayTime";	/* default service port		*/

	switch (argc) {
	case 1:
		host = "localhost";
		break;
	case 3:
		service = argv[2];
		/* FALL THROUGH */
	case 2:
		host = argv[1];
		break;
	default:
		printf( "usage: TCPdaytime [host [port]]\n");
	}

	TCPdaytime(host, service);
	return 0;
}

/*------------------------------------------------------------------------
 * TCPdaytime - invoke Daytime on specified host and print results
 *------------------------------------------------------------------------
 */
 int
TCPdaytime(const char *host, const char *service)
{
	char	buf[LINELEN+1];		/* buffer for one line of text	*/
	int	s, n;			/* socket, read count		*/

	s = connectTCP(host, service);

	while( (n = read(s, buf, LINELEN)) > 0) {
		buf[n] = '\0';		/* ensure null-terminated	*/
		(void) fputs( buf, stdout );
	}
	return 0;
}

 /* connectTCP.c - connectTCP */

 int	connectsock(const char *host, const char *service,
 		const char *transport);

 /*------------------------------------------------------------------------
  * connectTCP - connect to a specified TCP service on a specified host
  *------------------------------------------------------------------------
  */
 int
 connectTCP(const char *host, const char *service )
 /*
  * Arguments:
  *      host    - name of host to which connection is desired
  *      service - service associated with the desired port
  */
 {
 	return connectsock( host, service, "tcp");
 }
 /*------------------------------------------------------------------------
  * connectsock - allocate & connect a socket using TCP or UDP
  *------------------------------------------------------------------------
  */
 int
 connectsock(const char *host, const char *service, const char *transport )
 /*
  * Arguments:
  *      host      - name of host to which connection is desired
  *      service   - service associated with the desired port
  *      transport - name of transport protocol to use ("tcp" or "udp")
  */
 {
 	struct hostent	*phe;	/* pointer to host information entry	*/
 	struct servent	*pse;	/* pointer to service information entry	*/
 	struct protoent *ppe;	/* pointer to protocol information entry*/
 	struct sockaddr_in sin;	/* an Internet endpoint address		*/
 	int	s, type;	/* socket descriptor and socket type	*/


 	memset(&sin, 0, sizeof(sin));
 	sin.sin_family = AF_INET;

     /* Map service name to port number */
 	if ( (pse = getservbyname(service, transport) ))
 		sin.sin_port = pse->s_port;
 	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0){
 		printf("can't get \" %s\" service entry\n",service);
 	}

     /* Map host name to IP address, allowing for dotted decimal */
 	if ((phe = gethostbyname(host) ))
 		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
 	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE ){
 		printf("can't get \"%s\" host entry\n", host);
 	}
 	char* ppp = inet_ntoa(sin.sin_addr);

 	printf("localhost = %s\n",ppp);

     /* Map transport protocol name to protocol number */
 	if ( (ppe = getprotobyname(transport)) == 0){
 		printf("can't get \"%s\" protocol entry\n", transport);}

     /* Use protocol to choose a socket type */
 	if (strcmp(transport, "udp") == 0)
 		type = SOCK_DGRAM;
 	else
 		type = SOCK_STREAM;

 	/* Allocate a socket */ s = socket(PF_INET, type, ppe->p_proto);
 	if (s < 0){
 		printf("can't create socket:");
 		}

     /* Connect the socket */
 	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
 		printf("can't connect to %s.%s\n", host, service);}
 	return s;
 }
