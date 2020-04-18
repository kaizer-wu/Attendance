#ifndef __gpio_h__
#define __gpio_h__

#if 0
#define LIGHT_MODLE_RGB
#define Rpin 25
#define Gpin 6
#define Bpin 7

#else
#define LIGHT_MODLE_LED
#define LEDpin 4
#endif

#define KEYpin 3

#define LIGHT_POWE 0
#define LIGHT_WAIT 1
#define LIGHT_SUCC 2
#define LIGHT_FAIL 3
#define LIGHT_EROR 4
#define LIGHT_EXIT 5

int gpio_on();
void gpio_off();

#endif
