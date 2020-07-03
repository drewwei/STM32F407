#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"
#include "can.h"
/* can1ģ���ʼ�� 
 * PA11,PA12 CAN1����
 */
int CAN1_Mode_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	/* 1.ʹ��poartA�˿�ʱ�ӣ�ʹ��can1ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //��Ҫ��˯��ģʽ��ʱ��ʹ�ܺ���RCC_AHB1PeriphClockLPModeCmd
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//����Ϊ���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;	//����
	/* 2.����GPIO�����ú͵������� */
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��GPIO���õ�������
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);  //���ø�������
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);  //���ø�������
	/* 3.����CAN������ */
	
	/* baudrate = 1/T = PCLK/((sys(1) + tBS1 + tBS2 ) * (BRP[9:0]+1))  =42000000/(1+7+6)/3 = 1M
	 * T = sys(1) + tBS1 + tBS2 + tRJW
	 * tq = (BRP[9:0]+1) x (1/PCLK) 
	 * tBS1 = tCAN x (TS1[3:0] + 1) ; tBS2 = tCAN x (TS2[2:0] + 1); tRJW = tCAN x (SJW[1:0] + 1)
	 * AHB ������Ƶ��Ϊ 168 MHz������ APB2 ����������Ƶ��Ϊ 84 MHz������ APB1 ����������Ƶ
	 * ��Ϊ 42 MHz��
	 */
	//CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//�ػ�ģʽ
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;			// normal ģʽ
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
	
	CAN_Init(CAN1, &CAN_InitStructure);	//��ʼ�� can1
	/* 4.����ɸѡ�� */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;	//ѡ��ɸѡ��0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	//��ʶ������ģʽ
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;	//32λID
	CAN_FilterInitStructure.CAN_FilterIdLow  = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; //32λ��������
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  =0x0000;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	//32λģʽ
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;	//ʹ��ɸѡ��0;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //ָ������FIFO0�󶨸�ɸѡ�����Ժ���FIFO0��ȡ����

	CAN_FilterInit(&CAN_FilterInitStructure);
	
	return 0;
}



/* CAN �������ݣ�����λCAN������ ,����Ĭ�Ϸ�������֡*/
int CAN_SendMsg(CAN_TypeDef* CANx, unsigned char *msg, unsigned char len)
{
	if(!CANx) return -1;
	
	unsigned char mbox;
	int i;
	CanTxMsg TxMsg;
	
	TxMsg.StdId	= 0x12; //��׼ID 11bit 
	TxMsg.ExtId = 0x12; //��չID,29bit
	TxMsg.IDE   = 0; // ����0��ʾ��׼��ʶ��
	TxMsg.RTR   = 0; // Զ��֡����λ������0��ʾ����֡��1��ʾң��֡
	TxMsg.DLC   = len; // ���ݳ���λ
	for(i = 0; i < 8; i++)
	{
		TxMsg.Data[i] = msg[i];	//����λ0-8�ֽ�
	}
	
	mbox = CAN_Transmit(CANx, &TxMsg); //���ش���������
	
	i = 0;
	
	while(CAN_TransmitStatus(CANx, mbox) == CAN_TxStatus_Failed) 
	{
			if(i > 0xfff)	return -1;
			i++;
	}
	
	return 0;
}

/* CAN �������ݣ�����λCAN������,���ջ��棬���ؽ��յ����ֽ���������Ĭ�Ͻ�������֡*/
int CAN_ReciveMsg(CAN_TypeDef* CANx , unsigned char *buf)
{
	CanRxMsg RxMsg;
	int i;
	unsigned int count0, count1;
	count0 = CAN_MessagePending(CANx, CAN_FIFO0);
	count1 = CAN_MessagePending(CANx, CAN_FIFO1);
	if((count0 == 0) && (count1 == 0)) return -1;
	if(count0) 
	{
		CAN_Receive(CANx, CAN_FIFO0, &RxMsg);
	}else if(count1)
	{
		CAN_Receive(CANx, CAN_FIFO1, &RxMsg);
	}
	
	for(i = 0; i < RxMsg.DLC; i++)
	{
		buf[i] = RxMsg.Data[i];
	}
	
	return RxMsg.DLC;
}







