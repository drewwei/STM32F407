#include "stdio.h"
#include "string.h"
#include "common.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"

void UART1_Init(uint32_t baudrate)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //不要用睡眠模式的时钟使能函数RCC_AHB1PeriphClockLPModeCmd
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	GPIO_InitTypeDef GPIOA_InitStructure = {
		.GPIO_Pin 	= GPIO_Pin_9 | GPIO_Pin_10,
		.GPIO_Mode	= GPIO_Mode_AF,
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd	= GPIO_PuPd_UP,
	};

	USART_InitTypeDef USART1_InitStructure ={
		.USART_BaudRate 					 = baudrate,
		.USART_WordLength 				 = USART_WordLength_8b,
		.USART_StopBits				 		 = USART_StopBits_1,
		.USART_Parity 						 = USART_Parity_No,
		.USART_Mode 							 = USART_Mode_Rx|USART_Mode_Tx,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
	};
	NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannel 									 = USART1_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority				 = 3,
		.NVIC_IRQChannelCmd								 = ENABLE,
	};
	//初始化uart1中断优先级
	NVIC_Init(&NVIC_InitStructure);
	
	
	//配置GPIOA9,10为复用模式
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource9 | GPIO_PinSource10  , GPIO_AF_USART1); 错误，不能与，因为不是按位计算的
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);  //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	//配置GPIOA9,10
	GPIO_Init(GPIOA, &GPIOA_InitStructure);
	//配置UART1
	USART_Init(USART1, &USART1_InitStructure);
	//使能UART1
	USART_Cmd(USART1,ENABLE);
	/*清中断避免已开启中断就产生中断*/
	USART_ClearFlag(USART1, USART_FLAG_TC);
	//使能uart1的接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	/**使能usart1的DMA发送*/
	//USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);	
}

void UART1_SendByte(uint8_t data)
{
	/* 发送数据寄存器为空时才可将数据写入发送数据寄存器，不然之前的数据会被覆盖*/
	while((USART1->SR & 0X40) == 0);
	USART1->DR = (u8)data;
}
int fputc(int ch, FILE *f)   //重定向，让printf输出到串口  
{
    UART1_SendByte(ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	
    return ch;
}

void UART1_SendStr(uint8_t * data)
{
	int i = 0;
	while(data[i])
	{
			if(data[i] != '\0')
			{
					UART1_SendByte(data[i]);
					i++;
			}		
	}
}

void UART1_RecvByte(uint8_t *data)
{
	/* 读数据寄存器为非空时才可将从读数据寄存器读出数据*/
	while((USART1->SR & (1<<5)) == 0);
	*data = USART1->DR;
}

uint8_t buffer[30];

#if 1  //我写的,敲击enter键发送字符串。优点：1、避免使用全局变量，2、省去在字符串开头结束加上额外的字符
void USART1_IRQHandler(void)
{
	/*如果时接收中断的话进入该分支*/
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)		
	{
			static int i = 0;
			uint8_t data;
			data = USART1->DR;
	/*当接收到回车换行时表示数据接收完成*/
		if(data == '\r' || data == '\n')								
			{
					buffer[i] = '\0';
					if(data == '\n') 
					{
							UART1_SendStr(buffer);
							UART1_SendStr("\r\n");
							if(strcmp("Turn_on_led", (char *)buffer) == 0)			  GPIO_ResetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15); //打开led
							else if(strcmp("Turn_off_led", (char *)buffer) == 0)  GPIO_SetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);   //关led
							i = 0;		
					}			
			}
			else {
							buffer[i++] = data;
			}
	/*清挂起，stm32f4串口接收中断标志位在数据被读出后硬件自动清除，可以不写这行*/
			USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	 }
		
}
#else //别人写的
int uart_byte_count;
void USART1_IRQHandler(void)  
{
	u8 rec_data;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //½ÓÊÕÖÐ¶Ï 
		{
				rec_data =(u8)USART_ReceiveData(USART1);         //(USART1->DR) ¶ÁÈ¡½ÓÊÕµ½µÄÊý¾Ý
      			if(rec_data=='S')		  	                         //Èç¹ûÊÇS£¬±íÊ¾ÊÇÃüÁîÐÅÏ¢µÄÆðÊ¼Î»
				{
					uart_byte_count=0x01; 
				}

			else if(rec_data=='E')		                         //Èç¹ûE£¬±íÊ¾ÊÇÃüÁîÐÅÏ¢´«ËÍµÄ½áÊøÎ»     ÆôÃ÷ÐÀÐÀ±àÐ´
				{
					if(strcmp("Light_led1",(char *)buffer)==0)        GPIO_ResetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15); //打开led
					else if(strcmp("Close_led1",(char *)buffer)==0)   GPIO_SetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);   //关led
					for(uart_byte_count=0;uart_byte_count<32;uart_byte_count++)buffer[uart_byte_count]=0x00;
					uart_byte_count=0;    
				}				  
			else if((uart_byte_count>0)&&(uart_byte_count<=30))
				{
				   buffer[uart_byte_count-1]=rec_data;
				   uart_byte_count++;
				}                		 
   } 
} 

#endif

