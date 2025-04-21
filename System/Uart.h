#ifndef MY_UART_H__
#define MY_UART_H__
#include "stm32f10x.h"  
void USART2_Config(uint32_t bound);
void USART_DMA_Init(void);
void DMA_USART2_Send(uint8_t *data, u16 size);

#endif

