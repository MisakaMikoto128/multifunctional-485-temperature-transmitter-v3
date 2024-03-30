/**
 * @file HDL_G4_Uart_test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-05
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_G4_Uart_test.h"
#include "HDL_G4_CPU_Time.h"
#include "log.h"
/**
 * @brief 串口测试方法，该函数会一直阻塞执行
 *
 */
void HDL_G4_Uart_test_loop()
{
	uint8_t buf[10] = {0};
	uint32_t data_len = 0;

	Uart_Init(COM1, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	//    Uart_Init(COM2, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	//    Uart_Init(COM3, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	//    Uart_Init(COM4, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	//    Uart_Init(COM5, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	//    Uart_Init(COM6, 1500000, LL_LPUART_DATAWIDTH_8B, LL_LPUART_STOPBITS_1, LL_LPUART_PARITY_NONE);
	while (1)
	{
		HAL_Delay(500);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		for (uint8_t com = COM1; com <= COM1; com++)
		{
			data_len = Uart_Read(com, buf, sizeof(buf));
			if (data_len > 0)
			{
				Uart_Write(com, buf, data_len);
			}
		}
	}
}

int stdout_putchar(int ch)
{
	while (!LL_USART_IsActiveFlag_TC(USART1))
	{
	}
	USART1->TDR = ch;
	return ch;
}

int stdout_putchar_have_fifo(int ch)
{
	while (!LL_USART_IsActiveFlag_TXE_TXFNF(USART1))
	{
	}
	USART1->TDR = ch;
	return ch;
}

void HDL_G4_Uart_fifo_test_loop()
{
	HDL_G4_CPU_Time_Init();
	Uart_Init(COM1, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	Uart_Init(COM2, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	Uart_Init(COM3, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
	
	int one_sample_test_times = 100;
	uint32_t start_tick = 0;

	uint32_t end_tick = 0;
	uint32_t total_tick = 0;
	uint8_t buf[16] = {0};

		for(int BLK = 1; BLK < 30; BLK++)
		{
			#define TEST_SIEZ 100

			one_sample_test_times = 100;
			total_tick = 0;

			while(one_sample_test_times > 0)
			{
				start_tick = HDL_G4_CPU_Time_GetUsTick();
				// for(int j = 0; j < BLK; j++)
				// {
				// 	// stdout_putchar_have_fifo('\0');
				// 	// stdout_putchar('\0');
				// }
				Uart_Write(COM2,buf,BLK);
				end_tick = HDL_G4_CPU_Time_GetUsTick();
	
				total_tick += end_tick - start_tick;
				HAL_Delay(10);
				one_sample_test_times--;
			}
	
			Debug_Printf("have fifo time %d:%d:%u:%f\r\n",TEST_SIEZ,BLK,total_tick,BLK*1.0f*TEST_SIEZ/total_tick);
			HAL_Delay(30);
		}
	

	// //UART TC中断测试
	// while (1)
	// {

	// 	HAL_Delay(1000);
	// 	for (int j = 0; j < 8; j++)
	// 	{
	// 		stdout_putchar_have_fifo('\0');
	// 	}
	// 	LL_USART_EnableIT_TC(USART1);
	// }
}
