#ifndef 	__ADC
#define		__ADC
#include "stm32f4xx.h"
extern void MY_ADC1_Config(void);
extern void Start_ADC1_Conv(void);
extern float ReadADCAverageValue(uint8_t channel);
extern void My_ADC_Enable(ADC_TypeDef* ADCx, FunctionalState NewState);

#define Channel_Num  9 																					// ＡＤＣ的通道数，本例使用9个通道
#define Sample_Num  10																					// 采样次数，本例使用平均滤波，采样10次取均值
extern uint16_t ADC_ConvertedValue[Sample_Num][Channel_Num];		// ADC采集数据的缓存
#endif
