#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"
#include "can.h"
/* can2模块初始化 
 * PB12,PB13 CAN1引脚
 */

int CAN2_Mode_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	
	/* 1.使能poartB端口时钟，使能can1时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //不要用睡眠模式的时钟使能函数RCC_AHB1PeriphClockLPModeCmd
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2 | RCC_APB1Periph_CAN1, ENABLE);	//使能CAN时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	//设置为复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;	//上拉
	/* 2.设置GPIO，复用和电气属性 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化GPIO设置电气属性
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);  //设置复用引脚  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);  //设置复用引脚
	/* 3.设置CAN控制器 */
	
	/* baudrate = 1/T = PCLK/((sys(1) + tBS1 + tBS2 ) * (BRP[9:0]+1))  =42000000/(1+7+6)/3 = 1M
	 * T = sys(1) + tBS1 + tBS2 + tRJW
	 * tq = (BRP[9:0]+1) x (1/PCLK) 
	 * tBS1 = tq x (TS1[3:0] + 1) ; tBS2 = tq x (TS2[2:0] + 1); tRJW = tq x (SJW[1:0] + 1)
	 * AHB 域的最大频率为 168 MHz。高速 APB2 域的最大允许频率为 84 MHz。低速 APB1 域的最大允许频
	 * 率为 42 MHz。
	 */
	//CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;	//回环模式
	CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;//CAN_Mode_Normal;			// normal 模式
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
	
	CAN_Init(CAN2, &CAN_InitStructure);	//初始化 can2
	#if 1
	/* 4.设置筛选器 */
	CAN_FilterInitStructure.CAN_FilterNumber = 14;	//选择筛选器0
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	//标识符屏蔽模式
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;	//32位ID
	CAN_FilterInitStructure.CAN_FilterIdLow  = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; //32位屏蔽掩码
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  =0x0000;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	//32位模式
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;	//使能筛选器14;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //指定分配FIFO0绑定该筛选器，以后往FIFO0读取数据

	CAN_FilterInit(&CAN_FilterInitStructure);	//CAN1,CAN2共用筛选器
	#endif
	return 0;
}

