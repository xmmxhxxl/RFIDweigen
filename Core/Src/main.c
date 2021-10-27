/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint32_t WG_Data;  //存放获取到的26位数据
uint8_t  WG_Count=0;  //存放获取数据的位数
uint8_t  WG_Status; //数据接收标志位
uint8_t  EVEN;
uint8_t  ODD;
uint8_t wg_recv[3];
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    //具体哪个串口可以更改USART1为其它串口
    while ((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕
    USART1->DR = (uint8_t) ch;
    return ch;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//	printf("WG_Count%d",WG_Count);
if(WG_Count==26)
{
WG_Status=1;
 __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);

}

HAL_TIM_Base_Stop_IT(&htim1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
 
	HAL_TIM_Base_Stop_IT(&htim1);
	TIM1->CNT=0;
	HAL_TIM_Base_Start_IT(&htim1);
	if(GPIO_Pin==GPIO_PIN_1)
	{ 
	  WG_Data <<= 1;
		WG_Count++;
	
	 __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
	
	}
	
	if(GPIO_Pin==GPIO_PIN_4)
	{ 
	  WG_Data <<= 1;
		WG_Data = WG_Data| 0x01;
		WG_Count++;
	
	 __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	
	}
 
 


}










int check_even_odd(uint8_t *str)
{
	uint8_t i, one_num	= 0;
	unsigned short check_temp;
	
		if(NULL == str)
		return -1;
 //偶校验
	check_temp = str[0];
	for(i = 0;i < 8;i++)
	{
		if(check_temp<<i & 0x80)
			one_num+=1;
	}	
	check_temp = str[1];
	for(i = 0;i < 4;i++)
	{
		if(check_temp<<i & 0x80)
			one_num+=1;
	}
	if(one_num % 2 )
		EVEN = 1;
	else
		EVEN = 0; 
	
	//奇校验
	one_num = 0;
	check_temp = str[1];
	for(i = 0;i < 4;i++)
	{
		if(check_temp>>i & 0x01)
			one_num+=1;
	}
	check_temp = str[2];
	for(i = 0;i < 8;i++)
	{
		if(check_temp<<i & 0x80)
			one_num+=1;
	}
	if(one_num % 2 )
		ODD = 0;
	else
		ODD = 1;	
	return 0;

}



int WG_Recv26(uint8_t *str)
{ 
		if(WG_Status){

			if(WG_Count == 26){
				str[0] = WG_Data>>17 & 0xFF;
				str[1] = WG_Data>>9  & 0xFF;
				str[2] = WG_Data>>1  & 0xFF;

				check_even_odd(str);
				if(EVEN == (WG_Data>>25 & 0x01) &&  ODD== (WG_Data & 0x01)){
					  WG_Status = 0;
						WG_Data = 0;
						WG_Count = 0;
					  
						return 1;
				}	
				else {
						WG_Status = 0;
						WG_Data = 0;
						WG_Count = 0;
						return 0;
				}
				
			
			}
			WG_Status = 0;
			WG_Data = 0;
			WG_Count = 0;
			return -1;
		}
		return -2;		
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
uint8_t sign=0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
__HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
HAL_TIM_Base_Start_IT(&htim1);
HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
				if(WG_Recv26(wg_recv)==1)
		{   
			HAL_UART_Transmit(&huart1, wg_recv,3, 10);
//				if((wg_recv[0]==0xE8)&&(wg_recv[1]==0xF5)&&(wg_recv[2]==0x51))
//			  {
			    
//					if(sign==0)__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,72);
//					else __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,36);
//					sign=!sign;
		
//				}
			 
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
