#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <signal.h>

#include "../include/cam.h"
#include "../include/sig.h"

extern struct jpg_buf_t *jpg;
extern int srlfd;

int Exitflag;

int client_init(char *ipaddr, unsigned short port, int backlog)
{
	int sockfd;
	int ret;
	struct sockaddr_in s_addr;//保存服务器的ip及port

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("socket");
		return -1;
	}
	printf("sockfd=%d\n",sockfd);

	memset(&s_addr,0,sizeof(s_addr));//内存清零函数
	s_addr.sin_family = AF_INET;//地址族：ipv4
	s_addr.sin_port = htons(port);//端口号由主机字节序转网络字节序
	s_addr.sin_addr.s_addr = inet_addr(ipaddr);//点分式转二进制网络字节序
	socklen_t s_len = sizeof(s_addr);


	//将服务器的ip和port与sockfd绑定
	ret = connect(sockfd, (struct sockaddr *)&s_addr,s_len);
	if(-1 == ret)
	{
		perror("connect");
		close(sockfd);
		return -1;
	}
	printf("connect success\n");

	
	return sockfd;
}

int server_wait_client_connect(int listenfd)
{
	int connfd;
	struct sockaddr_in clt_addr;
	socklen_t addrlen;

	/* 等待客户端的连接请求，并建立连接 */
	addrlen = sizeof(clt_addr);
	connfd = accept(listenfd, (struct sockaddr *)&clt_addr, &addrlen);
	if (connfd == -1) {
		perror("connfd");
		return -1;
	}

	printf("connfd = %d accept success\n", connfd);
	printf("IP : %s\n", inet_ntoa(clt_addr.sin_addr));
	printf("PORT : %d\n", ntohs(clt_addr.sin_port));

	return connfd;
}
#if 0

		if (strstr(cmd, "pic")) {
			/* 封装数据 */
			piclen = jpg->jpg_size;
			sprintf(cmd, "%dlen", piclen);
			/* 发送图像数据大小 */
			ret = write(connfd, cmd, sizeof(cmd));
			if (ret == -1) {
				perror("write->piclen");
				pthread_exit(NULL);
			}

			/* 发送图像数据内容 */
			count = 0;
			while(count < piclen) {
				ret = write(connfd, jpg->jpg_buf + count, jpg->jpg_size - count);
				if (ret == -1) {
					perror("write->pic");
					pthread_exit(NULL);
				}
				count += ret;
			}
#endif
int testFunc()
{
	int fd;
	int count;
	int ret;
	static int i = 1;
	char filename[16];

	sprintf(filename,"jpg%d.jpg",i);

	fd = open(filename,O_RDWR|O_CREAT,0666);

	while(!jpg->jpg_size);
	count=0;
	while(count<jpg->jpg_size)
	{
		ret = write(fd,jpg->jpg_buf+count,jpg->jpg_size-count);
		if(-1 == ret)
		{
			perror("write");
			return -1;
		}
		count += ret;
	}

	return 0;
}

void clientSigHandler(int sigid)
{
	
	switch(sigid)
	{
		case SIGBGIN:
			testFunc();
			break;
		case SIGSUCC:
			break;
		case SIGFAIL:
			break;
		case SIGEROR:
			break;
		case SIGEXIT:
			Exitflag=1;
			break;
	}
	
}

int client_on(void)
{
	int connfd;
	pthread_t pid;
	int ret;

	signal(SIGBGIN,clientSigHandler);
	signal(SIGSUCC,clientSigHandler);
	signal(SIGFAIL,clientSigHandler);
	signal(SIGEROR,clientSigHandler);
	signal(SIGEXIT,clientSigHandler);
	Exitflag = 0;

	while(!Exitflag) {
		printf("client on\n");
		sleep(5);
		// if (connfd == -1) {
		// 	exit(EXIT_FAILURE);
		// }

		// ret = pthread_create(&pid, NULL, client_process, (void *)&connfd);
		// if (ret != 0) {
		// 	fprintf(stderr, "pthread_create client fail\n");
		// 	return -1;
		// }

		// pthread_detach(pid);
	}
	return 0;
}
