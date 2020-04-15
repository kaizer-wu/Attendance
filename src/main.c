#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "../include/cam.h"
#include "../include/GPIO.h"
#include "../include/sig.h"

struct jpg_buf_t *jpg;

int main()
{
	pid_t pid;
	int shmid;
	int msgid;

	/* 共享内存的初始化 */
	shmid = shm_init(sizeof(struct jpg_buf_t), (void **)&jpg);
	jpg->jpg_size=0;
	//msgid = msg_init();

	/* 创建摄像头处理进程 */
	pid = fork();
	if (pid == -1) {
		perror("fork camera");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		camera_on();
		exit(EXIT_SUCCESS);
	}


	/* 创建串口处理进程 */
	pid = fork();
	if (pid == -1) {
		perror("fork GPIO");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		gpio_on();
		exit(EXIT_SUCCESS);
	}

	/* 创建客户端处理进程 */
	pid = fork();
	if (pid == -1) {
		perror("fork client");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		client_on();
		exit(EXIT_SUCCESS);
	}

	while(1) {
		printf("main process ......\n");
		sleep(10);
	}

	return 0;
}
