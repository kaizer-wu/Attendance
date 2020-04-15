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

#endif