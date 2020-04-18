#ifndef _sig_h_
#define _sig_h_

#include<signal.h>
#include<sys/types.h>
#include<unistd.h>

#define SIGBGIN SIGUSR1
#define SIGSUCC SIGUSR2
#define SIGFAIL SIGSYS
#define SIGEROR SIGPWR
#define SIGEXIT SIGTERM

#define LOGE(...)  { \
        printf("%-16s:%-4d: E :\t",__func__,__LINE__); \
        fflush(stdout); \
        perror(__VA_ARGS__);}
#define LOGD(...) { \
        printf("%-16s:%-4d: D :\t",__func__,__LINE__); \
        printf(__VA_ARGS__);}

int shm_init(int size, void **shmaddr);
int client_on();
#endif