#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>

int shm_init(int size, void **shmaddr)
{
	key_t key;
	int shmid;

	/* 1. 获取key  */
	key = ftok(".", 'a');
	if (key == -1) {
		perror("ftok");
		return -1;
	}

	/* 2. 申请共享内存 */
	shmid = shmget(key, size, IPC_CREAT | 0777);
	if (shmid == -1) {
		perror("shmget");
		return -1;
	}
	printf("shmid = %d\n", shmid);

	/* 3. 内存映射，将共享内存的地址映射给应用进程 */
	*shmaddr = shmat(shmid, NULL, 0);
	if (*shmaddr == NULL) {
		perror("shmaddr");
		return -1;
	}
	printf("%d - %p\n", __LINE__, *shmaddr);

	return shmid;
}

int shm_exit(int shmid, void *shmaddr)
{
	int ret;

	/* 解除映射 */
	ret = shmdt(shmaddr);
	if (ret == -1) {
		perror("shmdt");
		return -1;
	}

	ret = shmctl(shmid, IPC_RMID, NULL);
	if (ret == -1) {
		perror("shmctl");
		return -1;
	}
	return 0;
}

int msg_init()
{
	key_t key;
	int msgid;

	key = ftok(".","b");
	if (key == -1) {
		perror("ftok");
		return -1;
	}

	msgid = msgget(key,IPC_CREAT|06660);
	if(msgid == -1){
		perror("msgget");
		return -1;
	}

	printf("msgid = %d\n",msgid);
	return msgid;
}

int msg_exit(int msgid)
{
	int ret;

	ret = msgctl(msgid,IPC_RMID,NULL);
	if(ret == -1){
		perror("msgctl");
		return -1;
	}
	return 0;

}