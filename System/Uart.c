#include "Uart.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define USART_MAX_LEN 100
volatile uint16_t recv_length = 0;              // 接收帧数据的长度
uint8_t DMA_USART2_RX_BUF[USART_MAX_LEN] = {0}; // 接收数据缓存
uint8_t DMA_USART2_TX_BUF[400];                 // 发送数据缓存

void RS485_REN_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	 //R485EN//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	
	 GPIO_ResetBits(GPIOB,GPIO_Pin_15);
}


void USART2_Config(u32 bound) // 同时配置接收和发送
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    // 1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 2 GPIO USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);          // 初始化GPIOA.9

    // USART1_RX	  GPIOA.10初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;            // PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                // 初始化GPIOA.10
    USART_DeInit(USART2);

    // 3 中断  NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;  // 嵌套通道为DMA1_Channel4_IRQn
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; // 抢占优先级为 2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    // 响应优先级为 7
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 通道中断使能
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;  // 串口1发送中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);

    // 4配置 USART设置
    USART_InitStructure.USART_BaudRate = bound;                                     // 串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART2, &USART_InitStructure);                                       // 初始化串口1

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    USART_Cmd(USART2, ENABLE);
	
	RS485_REN_Init();
}

void USART_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    /* 开启DMA时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* 配置DMA1通道5（接收） */
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_USART2_RX_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART_MAX_LEN;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);

    /* 配置DMA1通道4（发送） */
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_USART2_TX_BUF;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0; // 发送时，BufferSize由实际数据大小决定
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    // 开启DMA发送完成中断
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    // 启用USART接收和发送DMA请求
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
		
		// 启动DMA接收通道
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

void DMA_USART2_Send(uint8_t *data, u16 size) // 串口1DMA发送函数
{
    // 禁用DMA通道
    DMA_Cmd(DMA1_Channel7, DISABLE);
	
	//拉高RS485REN
	 GPIO_SetBits(GPIOB,GPIO_Pin_15);
    // 清除所有标志位
    DMA_ClearFlag(DMA1_FLAG_TC7);
    DMA_ClearFlag(DMA1_FLAG_HT7);
    DMA_ClearFlag(DMA1_FLAG_TE7);
    USART_ClearFlag(USART2, USART_FLAG_TC);
    // 复制数据到发送缓冲区
    memcpy(DMA_USART2_TX_BUF, data, size);
    // 设置传输数据长度
    DMA_SetCurrDataCounter(DMA1_Channel7, size);
    // 使能DMA通道
    DMA_Cmd(DMA1_Channel7, ENABLE);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
	//拉低RS485REN
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
}

void USART2_IRQHandler(void) // 串口1中断服务程序
{
    /* 检查是否触发了空闲中断 (IDLE) */
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // 空闲中断触发
    {
        DMA_Cmd(DMA1_Channel6, DISABLE); /* 暂时关闭DMA接收，避免数据还未处理时进行错误的操作 */

        // 获取接收到的数据长度，单位为字节
        recv_length = USART_MAX_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);


        // 将接收到的数据复制到发送缓冲区并启动发送
        DMA_USART2_Send(DMA_USART2_RX_BUF, recv_length);

        // 重新设置DMA的计数器，确保下次DMA能够正确接收最大长度的数据
        DMA_SetCurrDataCounter(DMA1_Channel6, USART_MAX_LEN);

        DMA_Cmd(DMA1_Channel6, ENABLE); /* 重新启用DMA接收功能，继续接收数据 */
        USART_ReceiveData(USART2);      // 清除空闲中断标志位（此函数有清除标志位的作用）
		USART_ClearITPendingBit(USART2, USART_IT_IDLE); /* 清除空闲中断标志 */
    }
    /* 检查是否完成了串口数据发送 */
    if (USART_GetFlagStatus(USART2, USART_IT_TXE) == RESET) // 判断发送完成标志位（TXE）
    {
        USART_ITConfig(USART2, USART_IT_TC, DISABLE); /* 禁用发送完成中断（TC中断），表示发送已经完成 */
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC7))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7); // 清除传输完成中断标志位
        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA1_Channel7->CNDTR = 0;
		// 清除数据长度
        USART_ITConfig(USART2, USART_IT_TC, ENABLE); // 打开串口发送完成中断

    }
}

//重定向scanf
//int fgetc(FILE *f)
//{
//  uint8_t ch=0;
//  HAL_UART_Receive(&huart2,&ch,1,0xffff);
//  return ch;
//}
 
//重定向printf
int fputc(int ch,FILE *f)
{
  uint8_t temp[1]={ch};
  DMA_USART2_Send(temp,1);
  return ch;
}







