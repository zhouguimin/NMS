#include "stm32f10x.h"
#include "I2C.h"

#define I2C_TIMEOUT 10000

void IIC_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	I2C_InitTypeDef I2C_InitStruct;
	
	I2C_DeInit(I2C1);
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;       // 快速模式占空比
    I2C_InitStruct.I2C_ClockSpeed = 100000;               // 100kHz
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;                // 主模式不需要地址
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE);
}

//void I2C_WriteByte(uint8_t addr,uint8_t data)
//{
//     //FlagStatus bitstatus = RESET
//    while (I2C_GetFlagStatus(I2C1,  I2C_FLAG_BUSY));  //??I2C??????	
//    I2C_GenerateSTART(I2C1,  ENABLE); //??I2C1 
//    //ErrorStatus status = ERROR,   ERROR??????,??0        
//    while( !I2C_CheckEvent(I2C1,  I2C_EVENT_MASTER_MODE_SELECT)); //EV5,???	
//	 I2C_Send7bitAddress(I2C1,OLED_ADDRESS, I2C_Direction_Transmitter); //??STM32?IIC???????,?????IIC???????????????,?????????
//	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));   //EV6
//	 I2C_SendData(I2C1,  addr);   //?????
//     while(!I2C_CheckEvent(I2C1,  I2C_EVENT_MASTER_BYTE_TRANSMITTING));  //EV8,?????? 
//     ??
//	I2C_SendData(I2C1,  data);   //????
//	 while(!I2C_CheckEvent(I2C1,  I2C_EVENT_MASTER_BYTE_TRANSMITTING)); //????????, EV8,????????
//    I2C_GenerateSTOP( I2C1,  ENABLE); //??I2C??
//}

uint8_t I2C1_Start(uint8_t address, uint8_t direction) 
{
    uint32_t timeout = I2C_TIMEOUT;

    //发送起始条件
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) return 0;
    }

    //发送设备地址和方向
    I2C_Send7bitAddress(I2C1, address, direction);
    timeout = I2C_TIMEOUT;
    if (direction == I2C_Direction_Transmitter) 
	{
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) 
		{
            if (--timeout == 0) return 0;
        }
    } 
	else 
	{
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) 
		{
            if (--timeout == 0) return 0;
        }
    }
    return 1;
}

void I2C1_Stop(void) {
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t I2C1_Write(uint8_t data) 
{
    uint32_t timeout = I2C_TIMEOUT;
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) 
		{
        if (--timeout == 0) return 0;
    }
    return 1;
}

uint8_t I2C1_Read(uint8_t ack) 
{
    uint32_t timeout = I2C_TIMEOUT;
    I2C_AcknowledgeConfig(I2C1, ack);
    if (!ack) I2C1_Stop(); //最后字节发送NACK后停止

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
        if (--timeout == 0) return 0xFF;
    }
    return I2C_ReceiveData(I2C1);
}

//向设备写数据
uint8_t I2C1_WriteBuffer(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len) 
{
    if (!I2C1_Start(devAddr, I2C_Direction_Transmitter)) return 0;
    if (!I2C1_Write(regAddr)) return 0;
    while (len--) {
        if (!I2C1_Write(*data++)) return 0;
    }
    I2C1_Stop();
    return 1;
}
//向设备读数据
uint8_t I2C1_ReadBuffer(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len) 
{
    if (!I2C1_Start(devAddr, I2C_Direction_Transmitter)) return 0;
    if (!I2C1_Write(regAddr)) return 0;
    if (!I2C1_Start(devAddr, I2C_Direction_Receiver)) return 0;

    while (len--) 
	{
        *data++ = I2C1_Read(len ? I2C_Ack_Enable : I2C_Ack_Disable);
    }
    return 1;
}
