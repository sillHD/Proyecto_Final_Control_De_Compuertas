/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "keypad.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ring_buffer.h"
#include "button.h"
#include "prints_display.h"
#include "command.h"
#include "display_main.h"
#include "state_machine.h"

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
ring_buffer_t rb_matrix;
ring_buffer_t rb_pc;
ring_buffer_t rb_internet;

uint8_t buffer_matrix[5];  // Buffer de tamaño 10 para almacenar el comando completo
uint8_t buffer_pc[5];
uint8_t buffer_internet[5];

uint32_t key_pressed_tick = 0;
uint16_t column_pressed = 0;
uint8_t b1_press_count = 0;


uint32_t debounce_tick = 0;
bool control_uart2 = false;
bool control_uart3 = false;
uint8_t key;

extern char state[3];
char state[3] = "Cl";

extern volatile uint32_t door_open_time; // Almacena el tiempo en el que se abrió la puerta temporalmente
extern volatile uint8_t temp_open;       // Flag para saber si la puerta está temporalmente abierta


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int system_events_handler(char *event)
{
  if (strcmp(event, "Op") == 0) {
    return 2;
  }
  else if (strcmp(event, "To") == 0) {
    return 1;
  }
  else if (strcmp(event, "Cl") == 0) {
    return 0;
  }
  return -1; // Return a default value if no condition is met
}

void low_power_sleep_mode(void) {
  HAL_SuspendTick(); // Detener el SysTick para reducir consumo
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick(); // Restaurar el SysTick después de salir del Sleep Mode
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == B1_Pin) { // Botón
      int press_type = detect_button_press();
      
      if (press_type == 1) { // Un solo clic
          if (strcmp(state, "Cl") == 0) {
              strcpy(state, "Op");
              HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Abierta\r\n", 16, 100);
              HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
              ssd1306_Fill(Black);
              show_opened();
          } else {
              strcpy(state, "Cl");
              HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Cerrada\r\n", 16, 100);
              HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
              ssd1306_Fill(Black);
              show_closed();
          }
      } else if (press_type == 2) { // Doble clic
          if (strcmp(state, "To") == 0) {
            return;
          }
          strcpy(state, "To"); // Temporalmente abierta
          HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Abierta Temporalmente\r\n", 30, 100);
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
          ssd1306_Fill(Black);
          show_opened();
          
          // Iniciar temporizador en el loop principal
          initialize_temp();
      }
  }
  else { // Keypad
      column_pressed = GPIO_Pin;
  }
}

void check_door_timeout() {
  if (temp_open && (HAL_GetTick() - door_open_time >= 3000)) { // Espera 3 segundos
      strcpy(state, "Cl");
      HAL_UART_Transmit(&huart2, (uint8_t *)"Puerta Cerrada\r\n", 16, 100);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      ssd1306_Fill(Black);
      show_closed();
      temp_open = 0; // Reiniciar flag
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    // HAL_UART_Transmit(&huart3, huart->pRxBuffPtr, huart->RxXferSize, 1000);
    control_uart2 = true;  
  } 
  else if (huart->Instance == USART3) {
    // HAL_UART_Transmit(&huart2, huart->pRxBuffPtr, huart->RxXferSize, 1000);
    control_uart3 = true;  

  }
}

/**
 * @brief  Heartbeat function to blink LED2 every 1 second to indicate the system is running
*/

void heartbeat(void)
{
  static uint32_t last_heartbeat = 0;
  if (HAL_GetTick() - last_heartbeat > 1000)
  {
    last_heartbeat = HAL_GetTick();
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
  }
}

int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 10);
  HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, 10);

  return len;
}

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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);  // Desactiva el buffer de stdout
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen();


  for (unsigned int i = 0; i < sizeof(buffer_matrix); i++) {
    buffer_matrix[i] = '_';
  }  

  keypad_init();  // Inicializa el teclado matricial
  HAL_UART_Receive_IT(&huart2, buffer_pc , 5);
  HAL_UART_Receive_IT(&huart3, buffer_internet , 5);

  ring_buffer_init(&rb_matrix, buffer_matrix, 5);  // Inicializa el ring buffer
  ring_buffer_init(&rb_pc, buffer_pc, 5);
  ring_buffer_init(&rb_internet, buffer_internet, 5);
  HAL_UART_Transmit(&huart2, (uint8_t *)"Hello World\r\n", 13, 100);  // Envía el mensaje "Hello World"
  HAL_UART_Transmit_IT(&huart3, (uint8_t *)"Hello World\r\n", 13);  // Envía el mensaje "Hello World"
  ssd1306_WriteString("Door default: Cl", Font_7x10, White);
  ssd1306_UpdateScreen();
  HAL_Delay(1000); 
  ssd1306_Fill(Black);
  show_closed();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    heartbeat();
    check_door_timeout();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Procesa teclas del teclado matricial
    if (column_pressed != 0 && (key_pressed_tick + 5) < HAL_GetTick()) {
      key = keypad_scan(column_pressed);
      column_pressed = 0;
      if (key != 'E') {
          ring_buffer_write(&rb_matrix, key);
          uint8_t size = ring_buffer_size(&rb_matrix);
          printf("Key: %c, Buffer: %s, Size: %d\r\n", key, buffer_matrix, size);
          process_command(&rb_matrix, buffer_matrix, state);
          //system_state_machine(state);

      }
    }

    // Procesa teclas recibidas desde la PC
    if(control_uart2)
    {     
      control_uart2 = false;
      ring_buffer_write(&rb_pc, buffer_pc[0]);
      ring_buffer_write(&rb_pc, buffer_pc[1]);
      ring_buffer_write(&rb_pc, buffer_pc[2]);
      ring_buffer_write(&rb_pc, buffer_pc[3]);
      ring_buffer_write(&rb_pc, buffer_pc[4]);
      uint8_t size = ring_buffer_size(&rb_pc);
      printf("Terminal Buffer: %s, Size: %d\r\n",buffer_pc, size);
      process_command(&rb_pc, buffer_pc, state);
      //system_state_machine(state);



    }
    if(control_uart3)
    {
      control_uart3 = false;
      ring_buffer_write(&rb_internet, buffer_internet[0]);
      ring_buffer_write(&rb_internet, buffer_internet[1]);
      ring_buffer_write(&rb_internet, buffer_internet[2]);
      ring_buffer_write(&rb_internet, buffer_internet[3]);
      ring_buffer_write(&rb_internet, buffer_internet[4]);
      uint8_t size = ring_buffer_size(&rb_internet);
      printf("\nInternet Buffer: %s, Size: %d\r\n",buffer_internet, size);
      process_command(&rb_internet, buffer_internet, state);
      //system_state_machine(state);


    }
    system_state_machine(state);
    HAL_UART_Receive_IT(&huart2, buffer_pc , 5);
    HAL_UART_Receive_IT(&huart3, buffer_internet , 5);
    HAL_Delay(100);
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LD1_Pin|ROW_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ROW_2_Pin|ROW_4_Pin|ROW_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LD1_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : COLUMN_1_Pin */
  GPIO_InitStruct.Pin = COLUMN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(COLUMN_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : COLUMN_4_Pin */
  GPIO_InitStruct.Pin = COLUMN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(COLUMN_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COLUMN_2_Pin COLUMN_3_Pin */
  GPIO_InitStruct.Pin = COLUMN_2_Pin|COLUMN_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ROW_1_Pin */
  GPIO_InitStruct.Pin = ROW_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(ROW_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW_2_Pin ROW_4_Pin ROW_3_Pin */
  GPIO_InitStruct.Pin = ROW_2_Pin|ROW_4_Pin|ROW_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
