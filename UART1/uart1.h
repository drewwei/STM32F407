#ifndef 		__UART1__
#define			__UART1__

#include "stdint.h"
extern void UART1_Init(uint32_t baudrate);

extern void UART1_SendByte(uint8_t data);
extern void UART1_SendStr(char * data);

#endif
