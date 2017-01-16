
#ifndef __MCU_H
#define __MCU_H

#include <stddef.h>
#include "stm32l1xx.h"
#include <stm32l1xx_gpio.h>

#define nop()	asm("nop")

void delay_us(unsigned long us);
void delay(uint32_t x);

#endif
