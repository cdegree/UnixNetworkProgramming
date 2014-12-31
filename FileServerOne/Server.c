/*
 * Server.c
 *文件传输服务器
 *  Created on: 2014年12月26日
 *      Author: 陈学位
 */
#include<netinet/in.h> // sockaddr_in
#include<sys/types.h>  // socket
#include<sys/socket.h> // socket
#include<stdio.h>    // printf
#include<stdlib.h>   // exit
#include<string.h>   // bzero
#include<unistd.h>  //close
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define QLEN 1024
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
unsigned short portbase = 0;
int passiveTCP(const char *service, int qlen);
int passivesock(const char *service, const char *transport, int qlen);

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

	// 创建socket，若成功，返回socket描述符
	int server_socket_fd = passiveTCP(service, QLEN);

	while (1) {
		// 定义客户端的socket地址结构
		struct sockaddr_in client_addr;
		socklen_t client_addr_length = sizeof(client_addr);

		// 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
		// accept函数会把连接到的客户端信息写到client_addr中
		int new_server_socket_fd = accept(server_socket_fd,
				(struct sockaddr*) &client_addr, &client_addr_length);
		if (new_server_socket_fd < 0) {
			perror("Server Accept Failed:");
			break;
		}
		// recv函数接收数据到缓冲区buffer中
		char buffer[BUFFER_SIZE];
		char file_name[FILE_NAME_MAX_SIZE + 1];
		int n;
		while ( (n=read(new_server_socket_fd,  file_name, FILE_NAME_MAX_SIZE))>0) {
			file_name[n] = '\0';
			// 然后从buffer(缓冲区)拷贝到file_name中
			printf("%s\n", file_name);
			// 打开文件并读取文件数据
			FILE *fp = fopen(file_name, "r");
			if (NULL == fp) {
				printf("File:%s Not Found\n", file_name);
				char *msg ="FILE NOT FOUND";
				strcpy(buffer,msg);
				int length = 0;
				if (write(new_server_socket_fd,buffer,strlen(buffer))< 0) {
					perror("Message Send Filed\n");
				}
			} else {
				bzero(buffer, BUFFER_SIZE);
				int length = 0;
				// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
				while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp))
						> 0) {
					if (send(new_server_socket_fd, buffer, length, 0) < 0) {
						printf("Send File:%s Failed./n", file_name);
						break;
					}
					bzero(buffer, BUFFER_SIZE);
				}
				// 关闭文件
				fclose(fp);
				printf("File:%s Transfer Successful!\n", file_name);
			}
			break;
		}
		// 关闭与客户端的连接
		close(new_server_socket_fd);
	}
	// 关闭监听用的socket
	close(server_socket_fd);
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

