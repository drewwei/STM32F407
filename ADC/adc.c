#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stdio.h"
#include "common.h"
#include "dma.h"
#define Channel_Num  9 																// ＡＤＣ的通道数，本例使用9个通道
#define Sample_Num  10																// 采样次数，本例使用平均滤波，采样10次取均值
uint16_t ADC_ConvertedValue[Sample_Num][Channel_Num];	// ADC采集数据的缓存
uint16_t ADC_Value[Channel_Num]={0};									// 9个ADC通道的采样值

void ADC_RegularConfig_Channel(void)
{
		/*ADC规则通道配置,ADC1 的9个规则通道，采样时间为3个采用周期*/
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_112Cycles);	
		ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 5, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 6, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 7, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 8, ADC_SampleTime_112Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 9, ADC_SampleTime_112Cycles);
}
void MY_ADC1_Config(void)
{
	/*0.0使能ADC1的时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 				// 使能uart1时已经打开了GPIOA的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);					// 使能uart1时已经打开了GPIOC的时钟
	ADC_DeInit(); 																								// 复位ADC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);					// 使能ADC1的时钟
	
	ADC_InitTypeDef ADC_InitStructure = {
			.ADC_Resolution 					=	ADC_Resolution_12b,						// 分辨率12位
			.ADC_ScanConvMode 				=	ENABLE,												// 开启扫描转换模式，扫描模式用于扫描一组模拟通道，知道最后一个通道转换完成后才停止
			.ADC_ContinuousConvMode 	=	ENABLE,												// 不开启连续转换模式，在连续转换模式下,ADC 结束一个转换后立即启动一个新的转换。数据会覆盖。
			.ADC_ExternalTrigConvEdge	=	ADC_ExternalTrigConvEdge_None,// 可以不写，采用软件触发
		//.ADC_ExternalTrigConv			= 
			.ADC_DataAlign						= ADC_DataAlign_Right,					// ADC数据寄存器右对齐
			.ADC_NbrOfConversion			=	9,														// 待转换的规则通道数
	};	
			
	ADC_CommonInitTypeDef ADC_CommonInitStructure = {
			.ADC_Mode							=	ADC_Mode_Independent,							// 独立模式
			.ADC_Prescaler				=	ADC_Prescaler_Div4,								// ADCCLK = 168/2/4 MHz= 21MHz
		  .ADC_DMAAccessMode		=	ADC_DMAAccessMode_Disabled, 			// 不用多ADC的DMA模式
			.ADC_TwoSamplingDelay	=	ADC_TwoSamplingDelay_10Cycles,  	// 采样间隔10个采样周期
	};
	
	GPIO_InitTypeDef GPIO_InitStructure = {		
			.GPIO_Pin			= GPIO_Pin_0 | GPIO_Pin_1|GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7,  	                             		
			.GPIO_Mode  	= GPIO_Mode_AN,															// 模拟输入模式
			.GPIO_PuPd		= GPIO_PuPd_NOPULL,													// 无上下拉
	};
	//0.1设置GPIO
	GPIO_Init(GPIOA, &GPIO_InitStructure);												// 初始化GPIOA
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	//0.1设置GPIO
  GPIO_Init(GPIOC, &GPIO_InitStructure);												// 初始化GPIOC
	//0.2设置规则采样通道
	ADC_RegularConfig_Channel();
	
	//1.ADC初始化
	ADC_Init(ADC1, &ADC_InitStructure);		
	ADC_CommonInit(&ADC_CommonInitStructure);											// ADC通用寄存器初始化
	
	//2.配置ADC DMA模式
	ADC_DMACmd(ADC1, ENABLE);																			// 使能ADC DMA模式（单个ADC）
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);							// 使能后当传输最后一个数据（DMA_BufferSize变为0时）时再次触发DMA响应，	
	//3.给ADC上电
	ADC_Cmd(ADC1, ENABLE); 																			
}

void Start_ADC1_Conv(void)
{
	//uint16_t data;
	//while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET);  				// 等待转换完成
		ADC_SoftwareStartConv(ADC1); 																	// 软件触发ADC转换	
	//data = ADC_GetConversionValue(ADC1); 													// 获取ADC的值
	//ADC_ClearFlag(ADC1, ADC_FLAG_EOC);   													// 清除转换完成标志位				
}

void My_ADC_Enable(ADC_TypeDef* ADCx, FunctionalState NewState)
{
		ADC_Cmd(ADCx, NewState); 
		ADC_SoftwareStartConv(ADCx);
}
float ReadADCAverageValue(uint8_t channel)
{
	uint16_t i;
	uint32_t sum = 0;
	for(i = 0; i < Sample_Num; i++)
	{
			sum += ADC_ConvertedValue[i][channel];										// 将Sample_Num次的采样相加
	}
	sum /= Sample_Num;
	return (sum*3.3/4096);
}






