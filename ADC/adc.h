#ifndef 	__ADC
#define		__ADC
#include "stm32f4xx.h"
extern void MY_ADC1_Config(void);
extern void Start_ADC1_Conv(void);
extern float ReadADCAverageValue(uint8_t channel);
extern void My_ADC_Enable(ADC_TypeDef* ADCx, FunctionalState NewState);

#define Channel_Num  9 																					// ���ģõ�ͨ����������ʹ��9��ͨ��
#define Sample_Num  10																					// ��������������ʹ��ƽ���˲�������10��ȡ��ֵ
extern uint16_t ADC_ConvertedValue[Sample_Num][Channel_Num];		// ADC�ɼ����ݵĻ���
#endif
