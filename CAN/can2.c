#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"
#include "can.h"
/* can2ģ���ʼ�� 
 * PB12,PB13 CAN1����
 */

int CAN2_Mode_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	/* 1.ʹ��poartB�˿�ʱ�ӣ�ʹ��can1ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //��Ҫ��˯��ģʽ��ʱ��ʹ�ܺ���RCC_AHB1PeriphClockLPModeCmd
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2 | RCC_APB1Periph_CAN1, ENABLE);	//ʹ��CANʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//����Ϊ���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;	//����
	/* 2.����GPIO�����ú͵������� */
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��GPIO���õ�������
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);  //���ø�������  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);  //���ø�������
	/* 3.����CAN������ */
	
	/* baudrate = 1/T = PCLK/((sys(1) + tBS1 + tBS2 ) * (BRP[9:0]+1))  =42000000/(1+7+6)/3 = 1M
	 * T = sys(1) + tBS1 + tBS2 + tRJW
	 * tq = (BRP[9:0]+1) x (1/PCLK) 
	 * tBS1 = tq x (TS1[3:0] + 1) ; tBS2 = tq x (TS2[2:0] + 1); tRJW = tq x (SJW[1:0] + 1)
	 * AHB ������Ƶ��Ϊ 168 MHz������ APB2 ����������Ƶ��Ϊ 84 MHz������ APB1 ����������Ƶ
	 * ��Ϊ 42 MHz��
	 */
	//CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//�ػ�ģʽ
	CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;//CAN_Mode_Normal;			// normal ģʽ
	CAN_InitStructure.CAN_Prescaler = 6;
	CAN_InitStructure.CAN_BS1	= 6;
	CAN_InitStructure.CAN_BS2 = 5;
	CAN_InitStructure.CAN_SJW = 0;
	
	CAN_InitStructure.CAN_ABOM = ENABLE;	//�Զ������߹رչ��� 
	CAN_InitStructure.CAN_AWUM = ENABLE;	//�Զ�����ģʽʹ��
	CAN_InitStructure.CAN_ABOM = DISABLE; //ʱ�䴥��ͨ��ģʽ ��ֹ
	CAN_InitStructure.CAN_NART = ENABLE;  //�Զ��ط���֪����ȷ����
	CAN_InitStructure.CAN_RFLM = DISABLE; //���� FIFO ����ģʽ,����������֮ǰ����Ϣ
	CAN_InitStructure.CAN_TXFP = DISABLE;	//���� FIFO ���ȼ�,���ȼ�����Ϣ��ʶ��ȷ��
	
	CAN_Init(CAN2, &CAN_InitStructure);	//��ʼ�� can2
	#if 1
	/* 4.����ɸѡ�� */
	CAN_FilterInitStructure.CAN_FilterNumber = 14;	//ѡ��ɸѡ��0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	//��ʶ������ģʽ
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;	//32λID
	CAN_FilterInitStructure.CAN_FilterIdLow  = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; //32λ��������
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  =0x0000;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	//32λģʽ
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;	//ʹ��ɸѡ��14;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //ָ������FIFO0�󶨸�ɸѡ�����Ժ���FIFO0��ȡ����

	CAN_FilterInit(&CAN_FilterInitStructure);	//CAN1,CAN2����ɸѡ��
	#endif
	return 0;
}

