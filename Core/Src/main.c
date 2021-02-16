#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

void vtask_led_handler(void *params);
void vtask_button_handler(void *params);

static void prvSetupHardware(void);
static void prvSetupUart(void);
void prvSetupGPIO(void);
void printmsg(char *msg);
void rtos_delay(uint32_t delay_in_ms);

#define TRUE		1
#define FALSE		0

char usr_msg[250] = { 0 };
uint32_t notification_value = 0;

int main(void)
{
	RCC_DeInit();

	SystemCoreClockUpdate();

	prvSetupHardware();

	sprintf(usr_msg, "This is Demo of Task Notify APIs\r\n");
	printmsg(usr_msg);

	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	xTaskCreate(vtask_led_handler, "TASK-LED", 500, NULL, 2, &xTaskHandle1);

	xTaskCreate(vtask_button_handler, "TASK-BUTTON", 500, NULL, 2, &xTaskHandle2);

	vTaskStartScheduler();

	for(;;);

}

void vtask_led_handler(void *params)
{
	uint32_t current_notification_value = 0;
	while(1)
	{
		if(xTaskNotifyWait(0, 0, &current_notification_value, portMAX_DELAY) == pdTRUE)
		{
			GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
			sprintf(usr_msg, "Notification is received : Button press count : %ld\r\n", current_notification_value);
			printmsg(usr_msg);
		}
	}
}

void vtask_button_handler(void *params)
{
	while(1)
	{
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
		{
			rtos_delay(100);

			xTaskNotify(xTaskHandle1, 0x00, eIncrement);
		}
	}
}

static void prvSetupUart(void)
{
	GPIO_InitTypeDef GPIO_UART_Pin;
	USART_InitTypeDef UART2_Init;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	memset(&GPIO_UART_Pin, 0, sizeof(GPIO_UART_Pin));

	GPIO_UART_Pin.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_UART_Pin.GPIO_Mode = GPIO_Mode_AF;
	GPIO_UART_Pin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_UART_Pin);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	memset(&UART2_Init, 0, sizeof(UART2_Init));

	UART2_Init.USART_BaudRate = 115200;
	UART2_Init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	UART2_Init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	UART2_Init.USART_Parity = USART_Parity_No;
	UART2_Init.USART_StopBits = USART_StopBits_1;
	UART2_Init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &UART2_Init);

	USART_Cmd(USART2, ENABLE);
}

void prvSetupGPIO(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitTypeDef LedInit, ButtonInit;
	LedInit.GPIO_Mode = GPIO_Mode_OUT;
	LedInit.GPIO_OType = GPIO_OType_PP;
	LedInit.GPIO_Pin = GPIO_Pin_15;
	LedInit.GPIO_Speed = GPIO_Low_Speed;
	LedInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &LedInit);

	ButtonInit.GPIO_Mode = GPIO_Mode_IN;
	ButtonInit.GPIO_OType = GPIO_OType_PP;
	ButtonInit.GPIO_Pin = GPIO_Pin_0;
	ButtonInit.GPIO_Speed = GPIO_Low_Speed;
	ButtonInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &ButtonInit);

//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
//
//	EXTI_InitTypeDef ExtiInit;
//	ExtiInit.EXTI_Line = EXTI_Line0;
//	ExtiInit.EXTI_Mode = EXTI_Mode_Interrupt;
//	ExtiInit.EXTI_Trigger = EXTI_Trigger_Falling;
//	ExtiInit.EXTI_LineCmd = ENABLE;
//
//	EXTI_Init(&ExtiInit);
//
//	NVIC_SetPriority(EXTI0_IRQn, 5);
//	NVIC_EnableIRQ(EXTI0_IRQn);
}

static void prvSetupHardware(void)
{
	prvSetupGPIO();
	prvSetupUart();
}

void printmsg(char *msg)
{
	for(uint32_t i = 0; i < strlen(msg); i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		USART_SendData(USART2, msg[i]);
	}

	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);
}

void rtos_delay(uint32_t delay_in_ms)
{
	uint32_t current_tick_count = xTaskGetTickCount();

	//xTicksToWait = (xTimeInMs * configTICK_RATE_HZ)/1000
	uint32_t delay_in_ticks = (delay_in_ms * configTICK_RATE_HZ) / 1000;

	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
}
