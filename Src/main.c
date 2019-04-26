/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "dma.h"
#include "hrtim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern HRTIM_HandleTypeDef hhrtim1;
extern UART_HandleTypeDef huart2;
// definimos las variables a usar 
uint8_t recepcionCompleta; // valdrá 1 cuando termina de recivir la cantidad de caracteres pedidos y 0 en otro caso. Es para saber cuando se termina una recepcón.
uint8_t reciveBuffer[64]; // string de caracteres recibidos, será un puntero al usarlo en el Hal receive_DMA. La primrer posición de memoria será la coordenada ("x" o "y") y la segunda es la posición.
double  numeroRecivido = 0; //el valor de posición, y se obtiene de la posición reciveBuffer + 1 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t txStr[9] = "Recivido\n"; //variable para transmitir y saber cuando funciona la comunicación
	uint16_t pwmValor = 0; // valor de la pwm a usar como input (ancho), se usa el valor de numeroRecivido

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_HRTIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_HRTIM_SimplePWMStart_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1);
  HAL_HRTIM_SimplePWMStart_IT(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA2);
  /* USER CODE END 2 */
	// primero se abre la comunicación antes de entrar al while, allí una vez que se recivió y se seteó la pwm,
  //se vuelve a abrir el receive para que constantemente se pueda mandar datos.
  HAL_UART_Receive_DMA(&huart2 , reciveBuffer , 7);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	Cuando hay recepción, abajo (línea 194) se setea una interrupción para poner el valor de recepcionCompleta en 1. 
//	Entonces se activa el if de abajo.
    /* USER CODE END WHILE */
	  if(recepcionCompleta)
			{
//	Si el input tiene una x en el primer caracter, entonces el resto serán los valores del pwm, con la función atof se pasa dicho valor de char a int
				if(reciveBuffer[0] == 'X' || reciveBuffer[0] == 'x')
					{
						numeroRecivido = atof( (const char *) (reciveBuffer + 1) );
						if(numeroRecivido >= 0 &&  numeroRecivido < 0xFFDF)
						{
							pwmValor = (uint16_t) numeroRecivido;
							__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, pwmValor);
						}
// el valor tiene que sewr >= 0 y < que 0xFFDF para que funcione la pwm, sino no se setea.
				  }
		  memset((void *) reciveBuffer, 0 , 7);
		  HAL_UART_Transmit(&huart2, txStr, 9, 1000);
		  recepcionCompleta = 0;
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		  HAL_UART_Receive_DMA(&huart2 , reciveBuffer , 7);
	   }
    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1;
  PeriphClkInit.Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// interrupcion del timer de alta resolucion
void HAL_HRTIM_Compare1EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{


}
// interrupcion del receive del Huart
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	recepcionCompleta = 1;
}
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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
