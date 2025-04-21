#include <stdio.h>
#include "stm32f10x.h"  
#include "freertos_demo.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "Delay.h"
#include "Uart.h"
#include "I2C.h"
#include "BQ76920.h"
#include "string.h"

#define START_TASK_STACK 128
#define Task_Priority_1 1

TaskHandle_t start_task_handle;

#define START_TASK_LED_STACK 128
#define Task_Priority_LED 1
TaskHandle_t LED_task_handle;

#define START_TASK_UART_STACK 128
#define Task_Priority_UART 1
TaskHandle_t UART_task_handle;

#define START_TASK_I2C_STACK 128
#define Task_Priority_I2C 1
TaskHandle_t I2C_task_handle;

void LED(void* param)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//开启GPIO时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//设置GPIO输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	
	while(1)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_14);
		//Delay_ms(1000);
		vTaskDelay(500);
		GPIO_SetBits(GPIOB, GPIO_Pin_14);
		vTaskDelay(500);
	}
}

void UART_Task(void* param)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	USART2_Config(115200);
	USART_DMA_Init();
	//char string[] = "Hello World!\r\n";
	//DMA_USART2_Send((uint8_t*)string, sizeof("Hello World!\r\n") - 1); 
	//printf("hello world");
	while (1)
	{
		vTaskDelay(500);
	}
}

void I2C_Task(void* param)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	USART2_Config(115200);
	USART_DMA_Init();
	IIC_Init(); 
	BQ76920_Init();
	BQ76920_Wake_OFF();
	vTaskDelay(1000);
	BQ76920_Wake_UP();

	
	uint8_t BQ76920_addr = 0x08;
	uint8_t SYS_STAT_Regist = 0x00;
	printf("hello world");
	//uint8_t I2C1_ReadBuffer(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);
	uint8_t Result;
	uint8_t data[10];
	
	Result = I2C1_ReadBuffer(BQ76920_addr, SYS_STAT_Regist, data, sizeof(data));
	printf("I2C1_ReadBuffer Result = %d", Result);
	vTaskDelay(500);
	//printf("I2C1_ReadBuffer data = %d", data[0]);
	while (1)
	{
		vTaskDelay(500);
	}
}


void start_task(void* param)
{
	xTaskCreate( (TaskFunction_t) LED,
						(char*)"LED", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
						(configSTACK_DEPTH_TYPE) START_TASK_LED_STACK,
						(void*)NULL,
						(UBaseType_t) Task_Priority_LED,
						(TaskHandle_t *) &LED_task_handle);
						
//	xTaskCreate( (TaskFunction_t) UART_Task,
//					(char*)"UART_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
//					(configSTACK_DEPTH_TYPE) START_TASK_UART_STACK,
//					(void*)NULL,
//					(UBaseType_t) Task_Priority_UART,
//					(TaskHandle_t *) &UART_task_handle);
					
	xTaskCreate( (TaskFunction_t) I2C_Task,
					(char*)"I2C_Task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
					(configSTACK_DEPTH_TYPE) START_TASK_I2C_STACK,
					(void*)NULL,
					(UBaseType_t) Task_Priority_I2C,
					(TaskHandle_t *) &I2C_task_handle);
	vTaskDelete(NULL);//删除自身
}

void freertos_start(void)
{
	//1:创建一个启动任务
	xTaskCreate( (TaskFunction_t) start_task,
						(char*)"start_task", /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
						(configSTACK_DEPTH_TYPE) START_TASK_STACK,
						(void*)NULL,
						(UBaseType_t) Task_Priority_1,
						(TaskHandle_t *) &start_task_handle);
	//2：启动调度器
	vTaskStartScheduler();
}

