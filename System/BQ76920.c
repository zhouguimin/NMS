#include "stm32f10x.h"
#include "BQ76920.h"

#define BQ769xx_WAKE_ON()      GPIO_SetBits(GPIOB,GPIO_Pin_3);
#define BQ769xx_WAKE_OFF()     GPIO_ResetBits(GPIOB,GPIO_Pin_3);

void BQ76920_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//关闭JTAG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	//Wake//
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);
	
	//Power On GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_SetBits(GPIOB,GPIO_Pin_5);		
		
	 //ALERT//
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	
	 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	 GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void BQ76920_Wake_UP(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
}

void BQ76920_Wake_OFF(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);
}
