#include<stdio.h>
#include<wiringPi.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>

#include"../include/sig.h"
#include"../include/GPIO.h"

static int Timeflag;
int Exitflag;
pthread_t lightThread;
int lightType;

void *lightCtrlThread(void * arg)
{
	while(!Exitflag)
	{
#ifdef LIGHT_MODLE_RGB
#endif
#ifdef LIGHT_MODLE_LED
		switch(lightType)
		{
			case LIGHT_POWE:
				break;
			case LIGHT_WAIT:
				break;
			case LIGHT_SUCC:
				lightType=LIGHT_POWE;
				break;
			case LIGHT_FAIL:
				lightType=LIGHT_POWE;
				break;
			case LIGHT_EROR:
				break;
			case LIGHT_EXIT:
				break;
		}
	}
#endif
	return 0;
}

int lightCtrl(int light_type)
{
	if (lightThread == 0)
	{
		if (pthread_create(&lightThread,NULL,lightCtrlThread,NULL) !=0 )
		{
			LOGE("pthread_create");
			return -1;
		}
	}
	return -1;
}



void gpioSigHandler(int sigid)
{
	
	Timeflag=1;
	switch(sigid)
	{
		case SIGSUCC:
			lightCtrl(LIGHT_SUCC);
			break;
		case SIGFAIL:
			lightCtrl(LIGHT_FAIL);
			break;
		case SIGEROR:
			lightCtrl(LIGHT_EROR);
			break;
		case SIGEXIT:
			lightCtrl(LIGHT_EXIT);
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
	signal(SIGALRM,gpioSigHandler);
	signal(SIGSUCC,gpioSigHandler);
	signal(SIGFAIL,gpioSigHandler);
	signal(SIGEROR,gpioSigHandler);
	signal(SIGEXIT,gpioSigHandler);
	return 0;
}

int gpio_on()
{
	gpio_init();
	return keyListen();
}
 