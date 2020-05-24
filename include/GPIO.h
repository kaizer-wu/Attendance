#ifndef __gpio_h__
#define __gpio_h__

#define KEYpin 0

#define LIGHT_POWE 0
#define LIGHT_WAIT 1
#define LIGHT_SUCC 2
#define LIGHT_FAIL 3
#define LIGHT_EROR 4
#define LIGHT_EXIT 5

#define COM "/dev/ttyUSB"
int gpio_on();
void gpio_off();

#endif
