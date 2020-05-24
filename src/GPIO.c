#include<stdio.h>
#include<wiringPi.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<assert.h>
#include<termios.h>
#include<time.h>

#include"../include/sig.h"
#include"../include/GPIO.h"

extern int msgid;
static int Timeflag;
int Exitflag;
pthread_t lightThread;
int lightType;
int srlfd;

/*********************************************************************************  
* Description：  串口初始化
* Input:         devpath：串口设备文件路径
* Output:        baudrate：波特率
* Return:       无
* Others:       无。
**********************************************************************************/
int serial_init(const char *devpath, int baudrate)
{
	int fd = -1;
	struct termios oldtio, newtio;

	assert(devpath != NULL);

	fd = open(devpath, O_RDWR | O_NOCTTY); 
	if (fd == -1) {
		LOGE("serial->open");
		return -1;
	}

	tcgetattr(fd, &oldtio);		/* save current port settings */

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_lflag = 0;		/* set input mode (non-canonical, no echo,...) */

	newtio.c_cc[VTIME] = 40;	/* set timeout value, n * 0.1 S */
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	
	return fd;
}

/*********************************************************************************  
* Description：  串口接收
* Input:         fd：串口设备描述符
*				 count:接收数据长度
* Output:        buf：接收buf，用户提供，保证有效性。
* Return:       >=0 接收了的实际数据长度，<0表示失败。
* Others:       无。
**********************************************************************************/
ssize_t serial_recv(int fd, void *buf, size_t count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = read(fd, buf, count);
	if (ret == -1)
		LOGE("serial->send");

	return ret;
}

/*********************************************************************************  
* Description：  串口发送
* Input:         fd：串口设备描述符
*				 buf：发送buf，用户提供，保证有效性。
*				 count:发送数据长度
* Output:        无。
* Return:       >=0 发送了的实际数据长度，<0表示失败。
* Others:       无。
**********************************************************************************/
ssize_t serial_send(int fd, const void *buf, size_t count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = write(fd, buf, count);
	if (ret == -1)
		LOGE("serial->send");

	return ret;
}



/*********************************************************************************  
* Description：  串口退出
* Input:         fd：设备描述符
* Output:        无
* Return:       无
* Others:       无。
**********************************************************************************/
int serial_exit(int fd)
{
	if (close(fd)) {
		LOGE("serial->exit");
		return -1;
	}

	return 0;
}


void gpioSigHandler(int sigid)
{
	struct rcv_buf_t recvbuf;
	bzero(&recvbuf,sizeof(recvbuf));
	Timeflag=0;
	switch(sigid)
	{
		case SIGBGIN:
			LOGD("SIGBGIN\n");
			serial_send(srlfd,"W",1);
			break;
		case SIGSUCC:
			msgrcv(msgid,&recvbuf,RBUFSIZE, 1,IPC_NOWAIT);
			LOGD("SIGSUCC\n");
			LOGD("recvbuf:%ld-%s\n",recvbuf.type,recvbuf.rcv_buf);
			serial_send(srlfd,"S",1);
			serial_send(srlfd,recvbuf.rcv_buf,RBUFSIZE);
			break;
		case SIGFAIL:
			msgrcv(msgid,&recvbuf,RBUFSIZE, 2,IPC_NOWAIT);
			LOGD("SIGFAIL\n");
			LOGD("recvbuf:%ld-%s\n",recvbuf.type,recvbuf.rcv_buf);
			serial_send(srlfd,"F",1);
			serial_send(srlfd,recvbuf.rcv_buf,RBUFSIZE);
			break;
		case SIGEROR:
			msgrcv(msgid,&recvbuf,RBUFSIZE, 3,IPC_NOWAIT);
			LOGD("SIGEROR\n");
			LOGD("recvbuf:%ld-%s\n",recvbuf.type,recvbuf.rcv_buf);
			serial_send(srlfd,"E",1);
			serial_send(srlfd,recvbuf.rcv_buf,RBUFSIZE);
			break;
		case SIGEXIT:
			LOGD("SIGEXIT\n");
			Exitflag=1;
			break;
	}
	
}

int keyListen()
{
	while(1)
	{
		if((digitalRead(KEYpin) == 0) && Timeflag == 0)
		{
			Timeflag = 1;
			alarm(5);
			kill(0,SIGBGIN);
			break;
		}
		if (Exitflag == 1)
		{
			return 0;
		}
	}
	return -1;
}

int gpio_init()
{
	Timeflag = 0;
	Exitflag = 0;
	lightThread = 0;

	wiringPiSetup();
	pinMode(KEYpin,INPUT);
	pullUpDnControl(KEYpin,PUD_UP);

#ifdef LIGHT_MODLE_RGB
#endif
#ifdef LIGHT_MODLE_LED
	pinMode(LEDpin,OUTPUT);



#endif
	signal(SIGBGIN,gpioSigHandler);
	signal(SIGALRM,gpioSigHandler);
	signal(SIGSUCC,gpioSigHandler);
	signal(SIGFAIL,gpioSigHandler);
	signal(SIGEROR,gpioSigHandler);
	signal(SIGEXIT,gpioSigHandler);
	return 0;
}

void settime()
{

	struct timespec time;
	clock_gettime(CLOCK_REALTIME,&time);
	struct tm nowTime;
	localtime_r(&time.tv_sec,&nowTime);
	char current[15]={0};
	sprintf(current, "%04d%02d%02d%02d%02d%02d", 
	nowTime.tm_year + 1900, nowTime.tm_mon+1, nowTime.tm_mday,
	nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
	serial_send(srlfd,current,14);
}

int gpio_on()
{
	int i;
	for (i=0;i<5;i++)
	{
		char com[16]={0};
		sprintf(com,"%s%d",COM,i);
		srlfd=serial_init(com,B9600);
		if(srlfd > 0)
		{
			LOGD("srlfd=%d\n",srlfd);
			break;
		}
		LOGD("COM %s error,change com port\n",com);

	}
	settime();
	gpio_init();
	while(!Exitflag) {
		LOGD("GPIO process ......\n");
		keyListen();
		pause();
	}
	serial_exit(srlfd);
	LOGD("GPIO quit\n");
	return 0;
}
 