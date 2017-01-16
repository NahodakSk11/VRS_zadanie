#include <stddef.h>
#include "stm32l1xx.h"
#include <stm32l1xx_gpio.h>

#include <sys/stat.h>

#include "uart.h"

// inicializacia USART2
void InitUART()
{
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART2, USART_IT_TC, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

void USART_putch(char ch)
{
	if (ch == '\n') {
		while (~USART2->SR & USART_FLAG_TXE);
		USART_SendData(USART2, '\r');
	}

	while (~USART2->SR & USART_FLAG_TXE);
	USART_SendData(USART2, ch);
}

void USART_puts(char *s) {
	for (; *s; s++) USART_putch(*s); // iteruje cez string a vypise cez UART
}

// bool shit z netu aby fungoval printf cez UART
/*********************/
int _fstat(int fd, struct stat *pStat) {
	pStat->st_mode = S_IFCHR;
	return 0;
}

int _close(int a) {
	return -1;
}

int _write(int fd, char *pBuffer, int size) {
	for (int i = 0; i < size; i++) USART_putch(pBuffer[i]);
	return size;
}

int _isatty(int fd) {
	return 1;
}

int _lseek(int a, int b, int c) {
	return -1;
}
/*********************/
