/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tem_hum_press.h"
#include "key_control.h"
#include "alarm.h"
#include "usart_transmit.h"
#include "OLED_hal.h"
#include "OLED_hal_font.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;
 uint32_t last_display_tick = 0; // 屏幕刷新定时器
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n--- System Boot ---\r\n", 24, 100);
  AlarmData.temp = 15.0f;
  AlarmData.hum = 70.0f;
  AlarmData.press= 1.02f;

  AHT20_Init();
	BMP280_Init(&hi2c2);
	OLED_Init();
	OLED_Clear();
  uint32_t last_aht20_tick = 0;
  uint32_t last_bmp280_tick = 0;
  uint8_t last_state = 0;  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // 状态机后台任务,用来进行任务的切换
    Sensor_Task(); 
		KEY_Scan();
		alarm_check();
    // 2. 定时触发任务
    uint32_t current_tick = HAL_GetTick();

    if (current_tick - last_aht20_tick >= 500) 
    {
        last_aht20_tick = current_tick;
        GetAHT20Value();  
    }

    if (current_tick - last_bmp280_tick >= 700) 
    {
        last_bmp280_tick = current_tick;
        GetBMP280Value(); 
    }

//    // 3. 状态打印（调试用，可以看它一步步走）
//    if (i2c_state != last_state)
//    {
//        last_state = i2c_state;
//        uart_send_str("State->");
//        uart_send_int(i2c_state); 
//        uart_send_str("\r\n");
//    }

    // 4. 数据就绪打印
//    if (sensor_data.new_data_flag == 1)
//    {
//        sensor_data.new_data_flag = 0; 
//        uart_send_str("Data Ready: ");
//        uart_send_int(sensor_data.temp);
//        uart_send_str("C, ");
//        uart_send_int(sensor_data.hum);
//        uart_send_str("%, ");
//        uart_send_int(sensor_data.press * 1000); // 放大看气压
//        uart_send_str("atm\r\n");
//    }
    if (current_tick - last_display_tick >= 800) 
    {
        last_display_tick = current_tick;
        InterfaceChange(); 
			if (Interface == interface_alarm)
			{
				Alarm_Interface_Refresh();
			}
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
// 回调函数
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) 
{
    if(hi2c->Instance == I2C2) i2c_state = I2C_WAIT_TX_CPLT; 
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) 
{
    if(hi2c->Instance == I2C2) i2c_state = I2C_PROCESS_DATA; 
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) 
{
    if(hi2c->Instance == I2C2) i2c_state = I2C_WAIT_TX_CPLT; 
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) 
{
    if(hi2c->Instance == I2C2) i2c_state = I2C_PROCESS_DATA; 
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
