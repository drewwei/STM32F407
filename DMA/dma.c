#include "stm32f4xx_dma.h"
#include "stm32f4xx.h"
#include "string.h"
#include "stdio.h"
#include "misc.h"
#include "adc.h"
#include "common.h"

/*设置DMA2_Stream0_IRQn中断优先级*/	
static void Init_DMA2_Stream0_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure_DMA = {
		.NVIC_IRQChannel 										= DMA2_Stream0_IRQn,
		.NVIC_IRQChannelPreemptionPriority	=	2,
		.NVIC_IRQChannelSubPriority					=	1,
		.NVIC_IRQChannelCmd									=	ENABLE,
	};
	NVIC_Init(&NVIC_InitStructure_DMA);
}
static void MyDMA_Init(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t channel, uint32_t pAddr, uint32_t memAddr, uint32_t dir, uint32_t bufSize)
{
		DMA_InitTypeDef DMA_InitStructre = {			
		.DMA_Channel          		  = channel,													// DMA_Channel_4,                		
		.DMA_PeripheralBaseAddr	  	=	pAddr,														// (uint32_t)&USART1->DR,
		.DMA_Memory0BaseAddr   			= memAddr,													// (uint32_t)data,                 		                       		
		.DMA_DIR                		= dir,															// DMA_DIR_MemoryToPeripheral,                     		                       		
		.DMA_BufferSize         		= bufSize,													// strlen(data),              		                       	
		.DMA_PeripheralInc      		=	DMA_PeripheralInc_Disable,				// 外设地址固定
		.DMA_MemoryInc          		= DMA_MemoryInc_Enable,             // 内存地址增加         		
		.DMA_PeripheralDataSize 		= DMA_PeripheralDataSize_HalfWord,  // 外设数据宽度半字                   		
		.DMA_MemoryDataSize     		= DMA_MemoryDataSize_HalfWord,      // 内存数据宽度半字                		
		.DMA_Mode               		= DMA_Mode_Normal,									// DMA_Mode_Normal:触发一次DMA，结束后EN位硬件自动DISABLE，DMA不再接受响应；DMA_Mode_Circular:循环触发不会DISABLE EN位，和2440的reload类似                 
		.DMA_Priority           		= DMA_Priority_Medium,              // 优先级中等        		
		.DMA_FIFOMode           		=	DMA_FIFOMode_Disable,							// fifo禁用
		.DMA_FIFOThreshold      		= DMA_FIFOThreshold_HalfFull, 			// 忽略                      		
		.DMA_MemoryBurst        		= DMA_MemoryBurst_Single,           // 内存单次发送，有一个数据发一个数据，不会积攒到一定数量再发          	                       	     	
		.DMA_PeripheralBurst	      = DMA_PeripheralBurst_Single,       // 同上        	
	};
		DMA_Init(DMAy_Streamx, &DMA_InitStructre);
}

void MY_EnableDMA(DMA_Stream_TypeDef* DMAy_Streamx)
{
	/*再次判断DMA是否在传输*/
	while(DMA_GetCmdStatus(DMAy_Streamx) == ENABLE);
	/*使能DMA*/
	DMA_Cmd(DMAy_Streamx, ENABLE);
}

void MYDMA_Config(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t channel, uint32_t pAddr, uint32_t memAddr, uint32_t dir, uint32_t bufSize)
{	
	/*1.循环等待传输结束*/
	while(DMA_GetCmdStatus(DMAy_Streamx) == ENABLE);
	/*2.复位DMA设置*/
	DMA_DeInit(DMAy_Streamx);	
	/*3.设置DMA时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	/*4.初始化中断优先级*/
	Init_DMA2_Stream0_NVIC();
	/*5.初始化DMA*/
	MyDMA_Init(DMAy_Streamx, channel, pAddr, memAddr, dir, bufSize);
	/*6.清中断防止开启后立即触发*/
	DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);
	/*7.配置DMA2_Stream0的传输完成中断*/
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
}

/****************************************************************************
* 名    称: void DMA2_Stream0_IRQHandler(void)
* 功    能：DMA2_Stream0中断服务函数
* 入口参数：无
* 返回参数：无
* 说    明：ADC每两秒连续转换10次规则通道组（9个通道），连续转换10次后触发一次DMA，平均后UART输出
****************************************************************************/
void DMA2_Stream0_IRQHandler(void)
{
		ADC_Cmd(ADC1, DISABLE); 											// ADC断电
		DMA_Cmd(DMA2_Stream0, DISABLE);								// DMA断电
		/*重新部分初始化DMA*/
		MyDMA_Init(DMA2_Stream0, DMA_Channel_0, (uint32_t)&ADC1->DR, (uint32_t)ADC_ConvertedValue[0], DMA_DIR_PeripheralToMemory, Channel_Num*Sample_Num);	
		int i;
		printf("****************************************\r\n");
		for(i = 0; i < 9; i++)
			{
					printf("channel%d ADC is : %f.\r\n", (i+1), ReadADCAverageValue(i));
			}	
		mdelay(1000*2);	
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);		// 清挂起,此处挂起和ARM的pend寄存器不同
		MY_EnableDMA(DMA2_Stream0);														// 使能DMA	
		My_ADC_Enable(ADC1, ENABLE);													// ADC重新上电并使能转换		
}









