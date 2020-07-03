#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"
#include "can.h"
/* can1模块初始化 
 * PA11,PA12 CAN1引脚
 */
int CAN1_Mode_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	/* 1.使能poartA端口时钟，使能can1时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //不要用睡眠模式的时钟使能函数RCC_AHB1PeriphClockLPModeCmd
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//设置为复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;	//上拉
	/* 2.设置GPIO，复用和电气属性 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化GPIO设置电气属性
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);  //设置复用引脚
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);  //设置复用引脚
	/* 3.设置CAN控制器 */
	
	/* baudrate = 1/T = PCLK/((sys(1) + tBS1 + tBS2 ) * (BRP[9:0]+1))  =42000000/(1+7+6)/3 = 1M
	 * T = sys(1) + tBS1 + tBS2 + tRJW
	 * tq = (BRP[9:0]+1) x (1/PCLK) 
	 * tBS1 = tCAN x (TS1[3:0] + 1) ; tBS2 = tCAN x (TS2[2:0] + 1); tRJW = tCAN x (SJW[1:0] + 1)
	 * AHB 域的最大频率为 168 MHz。高速 APB2 域的最大允许频率为 84 MHz。低速 APB1 域的最大允许频
	 * 率为 42 MHz。
	 */
	//CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//回环模式
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;			// normal 模式
	CAN_InitStructure.CAN_Prescaler = 6;
	CAN_InitStructure.CAN_BS1	= 6;
	CAN_InitStructure.CAN_BS2 = 5;
	CAN_InitStructure.CAN_SJW = 0;
	
	CAN_InitStructure.CAN_ABOM = ENABLE;	//自动的总线关闭管理 
	CAN_InitStructure.CAN_AWUM = ENABLE;	//自动唤醒模式使能
	CAN_InitStructure.CAN_ABOM = DISABLE; //时间触发通信模式 禁止
	CAN_InitStructure.CAN_NART = ENABLE;  //自动重发送知道正确发送
	CAN_InitStructure.CAN_RFLM = DISABLE; //接收 FIFO 锁定模式,不锁定覆盖之前的消息
	CAN_InitStructure.CAN_TXFP = DISABLE;	//发送 FIFO 优先级,优先级由消息标识符确定
	
	CAN_Init(CAN1, &CAN_InitStructure);	//初始化 can1
	/* 4.设置筛选器 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;	//选择筛选器0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	//标识符屏蔽模式
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;	//32位ID
	CAN_FilterInitStructure.CAN_FilterIdLow  = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; //32位屏蔽掩码
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  =0x0000;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	//32位模式
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;	//使能筛选器0;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //指定分配FIFO0绑定该筛选器，以后往FIFO0读取数据

	CAN_FilterInit(&CAN_FilterInitStructure);
	
	return 0;
}



/* CAN 发送数据，参数位CAN控制器 ,这里默认发送数据帧*/
int CAN_SendMsg(CAN_TypeDef* CANx, unsigned char *msg, unsigned char len)
{
	if(!CANx) return -1;
	
	unsigned char mbox;
	int i;
	CanTxMsg TxMsg;
	
	TxMsg.StdId	= 0x12; //标准ID 11bit 
	TxMsg.ExtId = 0x12; //扩展ID,29bit
	TxMsg.IDE   = 0; // 设置0表示标准标识符
	TxMsg.RTR   = 0; // 远程帧请求位，设置0表示数据帧，1表示遥控帧
	TxMsg.DLC   = len; // 数据长度位
	for(i = 0; i < 8; i++)
	{
		TxMsg.Data[i] = msg[i];	//数据位0-8字节
	}
	
	mbox = CAN_Transmit(CANx, &TxMsg); //返回传输的邮箱号
	
	i = 0;
	
	while(CAN_TransmitStatus(CANx, mbox) == CAN_TxStatus_Failed) 
	{
			if(i > 0xfff)	return -1;
			i++;
	}
	
	return 0;
}

/* CAN 接收数据，参数位CAN控制器,接收缓存，返回接收到的字节数，这里默认接收数据帧*/
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







