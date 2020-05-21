#include "common.h"
#include "stdio.h"
#include "string.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include "uart1.h"
#include "led.h"
#include "dma.h"
#include "adc.h"
const char *data = "DMA test\r\n";

/**********************************
{channel1 , channel1,.....channel9}
{channel1 , channel1,.....channel9}
...............................
{channel1 , channel1,.....channel9}
**********************************/

int main(void)
{
	int i = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	MYdelay_init();
	LED_Init();
	UART1_Init(115200);
	/*STM32的DMA因为不能外设到外设，所以它的DMA请求（触发）也是其传输时的外设，似乎没有软件触发和硬件触发的概念，stm32太垃圾*/
	//MYDMA_Config(DMA2_Stream7, DMA_Channel_4, (uint32_t)&USART1->DR, (uint32_t)data, DMA_DIR_MemoryToPeripheral, strlen(data));
	MY_ADC1_Config();
	/* 1.设置DMA*/
	MYDMA_Config(DMA2_Stream0, DMA_Channel_0, (uint32_t)&ADC1->DR, (uint32_t)ADC_ConvertedValue[0], DMA_DIR_PeripheralToMemory, Channel_Num*Sample_Num);
	/* 2.使能DMA*/
	MY_EnableDMA(DMA2_Stream0);
	/* 3.开启ADC转换，*/
	Start_ADC1_Conv();
	
	GPIO_ResetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15); //打开led
	
 /* 如果是在非循环模式下配置数据流，传输结束后（即要传输的数据数目达到零），除非软件
	* 重新对数据流编程并重新使能数据流（通过将 DMA_SxCR 寄存器中的 EN 位置 1），否则
	*	DMA 即会停止传输（通过硬件将 DMA_SxCR 寄存器中的 EN 位清零）并且不再响应任何
	* DMA 请求。
	*/
	while(1)
	{
			
			/* 4.打印转换次数*/
			printf("Convert times:%d.\r\n", ++i);				
			mdelay(1000*2);
	}
	
	//return 0;
}








