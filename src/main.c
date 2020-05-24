#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "../include/cam.h"
#include "../include/GPIO.h"
#include "../include/sig.h"

struct jpg_buf_t *jpg;

static int Exitflag = 0;

int msgid;

void sigHandler(int sigid)
{
	switch(sigid)
	{
		case SIGINT:
			kill(0,SIGEXIT);
			Exitflag = 1;
			break;
		default:
			break;
	}
}

int main()
{
	pid_t pid;

	/* 共享内存的初始化 */
	shm_init(sizeof(struct jpg_buf_t), (void **)&jpg);
	jpg->jpg_size=0;
	msgid = msg_init();

	/* 创建摄像头处理进程 */
	pid = fork();
	if (pid == -1) {
		LOGE("fork camera");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		camera_on();
		exit(EXIT_SUCCESS);
	}


	/* 创建串口处理进程 */
	pid = fork();
	if (pid == -1) {
		LOGE("fork GPIO");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		gpio_on();
		exit(EXIT_SUCCESS);
	}

	/* 创建客户端处理进程 */
	pid = fork();
	if (pid == -1) {
		LOGE("fork client");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		client_on();
		exit(EXIT_SUCCESS);
	}

	signal(SIGBGIN,SIG_IGN);
	signal(SIGSUCC,SIG_IGN);
	signal(SIGFAIL,SIG_IGN);
	signal(SIGEROR,SIG_IGN);
	signal(SIGEXIT,SIG_IGN);
	signal(SIGINT,sigHandler);

	while(!Exitflag) {
		LOGD("main process ......\n");
		pause();
	}
	LOGD("main quit\n");

	return 0;
}
