/*  main author：陈学位 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int	CommunicateWork(const char *host, const char *service);
int	connectTCP(const char *host, const char *service);

#define	LINELEN		4096

/*------------------------------------------------------------------------
 * main - TCP client for  service connect work
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
	case 2:
			host = argv[1];
			break;
	case 3:
		service = argv[2];
		host = argv[1];
		/* FALL THROUGH */
		break;
	default:
		printf( "usage: Client [host [port]]\n");
	}
	//connect函数在connectTCP里
	CommunicateWork(host, service);
	return 0;
}

 int
 CommunicateWork(const char *host, const char *service)
{
	char	buf[LINELEN+1];		/* buffer for one line of text	*/
	int	s, n;			/* socket, read count		*/

	s = connectTCP(host, service);
	char input[LINELEN+1]; /* input from keyboard*/
/*	while( (n = read(s, buf, LINELEN)) > 0) {
		buf[n]='\0';
		printf("Received from Server message : %s\n",buf);
		fflush(stdout);
		break;
	}*/
	printf("input file name:");
	scanf("%s",input);
	input[strlen(input)]='\0';
	write(s,input,strlen(input));
	FILE *file;
	file = fopen(input,"w");
	double start = clock();
	int bitCount = 0;
	while( (n = read(s, buf, LINELEN)) > 0) {
		buf[n]='\0';
		bitCount += n;
		char *msg = "FILE NOT FOUND";
		if(strcmp(msg,buf)==0){
			printf("Received from Server message : %s\n",buf);
		}
		else {
			if(fwrite(buf,sizeof(char),n,file)<0){
				printf("File Write Error\n");
				break;
			}
		}
		fflush(stdout);
	}
	double end = clock();
	fclose(file);
	printf("Received File %s  %.3fMB Time cost %.3f s \n",input,(double)bitCount/1024/1024,(end-start)/ CLOCKS_PER_SEC );

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

 	//printf("localhost = %s\n",ppp);

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
