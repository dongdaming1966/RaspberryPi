//use #gpio readall 
//in console to get gpio map
//
//use #gcc gpio.c -lwiringPi
//to compile this program
//Author:	Wiring Pi
//Date:		2018.1
#include <wiringPi.h>
int main (void)
{
	wiringPiSetup () ;
	pinMode (0, OUTPUT) ;
	for (;;)
	{
		digitalWrite (0, HIGH) ; delay (1) ;
		digitalWrite (0,  LOW) ; delay (1) ;
	}
	return 0 ;
}
