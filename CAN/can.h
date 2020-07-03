#ifndef	_CAN1_H
#define _CAN1_H

#include "stm32f4xx_can.h"

int CAN1_Mode_Init(void);
int CAN2_Mode_Init(void);

int CAN_SendMsg(CAN_TypeDef* CANx, unsigned char *msg, unsigned char len);
int CAN_ReciveMsg(CAN_TypeDef* CANx , unsigned char *buf);



#endif

