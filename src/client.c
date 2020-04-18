#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include <signal.h>

#include "../include/cam.h"
#include "../include/sig.h"

extern struct jpg_buf_t *jpg;
extern int srlfd;

int Exitflag;
int Stopflag;

int client_init()
{
	int sockfd;
	int ret;
	struct sockaddr_in s_addr;//保存服务器的ip及port

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		LOGE("socket()");
		return -1;
	}
	LOGD("sockfd=%d\n",sockfd);

	memset(&s_addr,0,sizeof(s_addr));//内存清零函数
	s_addr.sin_family = AF_INET;//地址族：ipv4
	s_addr.sin_port = htons(PORT);//端口号由主机字节序转网络字节序
	s_addr.sin_addr.s_addr = inet_addr(IP);//点分式转二进制网络字节序
	socklen_t s_len = sizeof(s_addr);


	//将服务器的ip和port与sockfd绑定
	ret = connect(sockfd, (struct sockaddr *)&s_addr,s_len);
	if(-1 == ret)
	{
		LOGE("connect");
		close(sockfd);
		return -1;
	}
	LOGD("connect success\n");

	
	return sockfd;
}

#if 0
int server_wait_client_connect(int listenfd)
{
	int connfd;
	struct sockaddr_in clt_addr;
	socklen_t addrlen;

	/* 等待客户端的连接请求，并建立连接 */
	addrlen = sizeof(clt_addr);
	connfd = accept(listenfd, (struct sockaddr *)&clt_addr, &addrlen);
	if (connfd == -1) {
		LOGE("connfd");
		return -1;
	}

	LOGD("connfd = %d accept success\n", connfd);
	LOGD("IP : %s\n", inet_ntoa(clt_addr.sin_addr));
	LOGD("PORT : %d\n", ntohs(clt_addr.sin_port));

	return connfd;
}
#endif

int sendPic(int connfd)
{
	int ret;
	int count;
	char piclen[10] = {0};


	while(!Stopflag)
	{
		static int write_size = 0;
		if(jpg->jpg_size == 0 || write_size == jpg->jpg_size)
		{
			continue;
		}
		bzero(piclen,sizeof(piclen));
		write_size = jpg->jpg_size;
		sprintf(piclen,"%d",write_size);
		ret = write(connfd, piclen, sizeof(piclen));
		if (ret == -1) {
			LOGE("write(piclen)");
			pthread_exit(NULL);
		}
		LOGD("write piclen:%s\n",piclen);

		/* 发送图像数据内容 */
		count = 0;
		while(count < write_size) {
			ret = write(connfd, jpg->jpg_buf + count, write_size - count);
			if (ret == -1) {
				LOGE("write(picbuf)");
				return -1;
			}
			count += ret;
		}
		LOGD("write count:%d\n",count);
	}
	return 0;
}

void* clientRecvThread(void *arg)
{
	int connfd;
	int count;
	int ret;
	static int i = 1;
	char recvbuf[1] = {0};
	
	connfd = (int)arg;

	while(!Stopflag)
	{
		bzero(recvbuf,sizeof(recvbuf));
		ret = read(connfd,recvbuf,sizeof(recvbuf));
		if(ret == -1 && Stopflag == 1)
		{
			pthread_exit(NULL);
		}
		else if(ret == -1){
			LOGE("read()");
			pthread_exit(NULL);
			Stopflag = 1;
		}
		else if (ret == 0)
		{
			LOGD("server disconnected\n");
			kill(0,SIGEROR);
			pthread_exit(NULL);
		}
		else
		{
			switch(recvbuf[0])
			{
				case 'S':
					kill(0,SIGSUCC);
					break;
				case 'F':
					kill(0,SIGFAIL);
					break;
				case 'E':
					kill(0,SIGEROR);
					break;
				default: 
					break;
			}
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

void clientSigHandler(int sigid)
{
	static int connfd;
	static pthread_t pthread = 0;
	int ret;
	switch(sigid)
	{
		case SIGBGIN:
			if(Stopflag == 0) break;
			Stopflag = 0;
			connfd = client_init();
			if(connfd > 0)
			{
				if(pthread == 0)
				{
					ret = pthread_create(&pthread,NULL,clientRecvThread,(void*)connfd);
					if(ret == -1)
					{
						LOGE("pthread_create()");
					}
				}
				sendPic(connfd);
			}
			break;
		case SIGSUCC:
			Stopflag = 1;
			break;
		case SIGFAIL:
			Stopflag = 1;
			break;
		case SIGEROR:
			Stopflag = 1;
			break;
		case SIGEXIT:
			Stopflag = 1;
			Exitflag = 1;
			break;
	}
	close(connfd);
	pthread_join(pthread,NULL);
	LOGD("pthread exit\n");
}

int client_on()
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
	Stopflag = 1;

	while(!Exitflag) {
		LOGD("client process..\n");
		pause();
	}
	return 0;
}
