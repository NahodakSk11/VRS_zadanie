#include "mcu.h"

void delay_us(unsigned long us)
{
	int i, tus;
	tus = 9 * us;
	for(i = 0; i < tus; i++) nop();
}

void delay(uint32_t x)
{
    while (x--);
}
