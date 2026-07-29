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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DS18B20_PORT GPIOB
#define DS18B20_PIN GPIO_PIN_8
#define MAX485_RX_EN_PORT GPIOB
#define MAX485_RX_EN_PIN GPIO_PIN_3
#define MAX485_TX_EN_PORT GPIOA
#define MAX485_TX_EN_PIN GPIO_PIN_15

#define ADC_VREF        3.3f
#define ADC_FULLSCALE   4095.0f
#define TDS_FACTOR_K    0.5f     // 0.5 padrão (NaCl). Ajuste na calibração.
#define ALPHA_TEMP      0.02f    // coef. térmico da condutividade

#define SLOPE_25_mV_pH  59.16f   // @25°C
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint16_t v0, v1, v2;
volatile float a0, a1, a2, NTU, NTU_regressao;
volatile float NTU_mul = 2.881, NTU_lin = 538;
float Temperature = 0;
float Humidity = 0;
uint8_t Presence = 0;
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;

// Variaveis de TDS
float TDS_ppm;         // ppm (mg/L)
float EC_uScm;         // µS/cm
float EC_mScm;         // mS/cm
volatile float mScm_regressao;
volatile float mScm_mul = 1.37;

// Variaveis de pH
float v_ph;
float pH_value;
float K_DIV = 0.68;

// Variavel de recepção da UART
volatile uint8_t rx_byte;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static HAL_StatusTypeDef ADC1_ReadOne(uint32_t channel,
                                      uint32_t sample,   // ex: ADC_SAMPLETIME_13CYCLES_5
                                      uint16_t *out)
{
    HAL_StatusTypeDef st;
    ADC_ChannelConfTypeDef s = {0};

    // Garante que não está convertendo
    HAL_ADC_Stop(&hadc1);

    s.Channel      = channel;
    s.Rank         = ADC_REGULAR_RANK_1;    // com NbrOfConversion=1
    s.SamplingTime = sample;

    st = HAL_ADC_ConfigChannel(&hadc1, &s);
    if (st != HAL_OK) return st;

    st = HAL_ADC_Start(&hadc1);
    if (st != HAL_OK) return st;

    st = HAL_ADC_PollForConversion(&hadc1, 10);
    if (st != HAL_OK) { HAL_ADC_Stop(&hadc1); return st; }

    *out = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return HAL_OK;
}

float ntu_from_adc(uint16_t raw)
{
    const float VREF = 3.3f;
    float Vadc = (float)raw * VREF / 4095.0f;  // tensão no pino do ADC (depois do divisor)

    // Polinômio já “absorvendo” o fator 0,66 do divisor
    return -2572.08448f*Vadc*Vadc + 8700.45455f*Vadc - 4352.9f;
}

void delay (uint16_t time)
{
	/* change your code here for the delay in microseconds */
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim2))<time);
}

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

uint8_t DS18B20_Start (void)
{
	uint8_t Response = 0;
	Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);   // set the pin as output
	HAL_GPIO_WritePin (DS18B20_PORT, DS18B20_PIN, 0);  // pull the pin low
	delay (480);   // delay according to datasheet

	Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);    // set the pin as input
	delay(80);
	if (!(HAL_GPIO_ReadPin (DS18B20_PORT, DS18B20_PIN))) Response = 1;    // if the pin is low i.e the presence pulse is detected
	else Response = -1;

	delay (400); // 480 us delay totally.

	return Response;
}

void DS18B20_Write (uint8_t data)
{
	Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);  // set as output

	for (int i=0; i<8; i++)
	{
		if ((data & (1<<i))!=0)  // if the bit is high
		{
			// write 1
			Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);  // set as output
			HAL_GPIO_WritePin (DS18B20_PORT, DS18B20_PIN, 0);  // pull the pin LOW
			delay (1);  // wait for 1 us

			Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);  // set as input
			delay (50);  // wait for 60 us
		}

		else  // if the bit is low
		{
			// write 0
			Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
			HAL_GPIO_WritePin (DS18B20_PORT, DS18B20_PIN, 0);  // pull the pin LOW
			delay (50);  // wait for 60 us

			Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
		}
	}
}

uint8_t DS18B20_Read (void)
{
	uint8_t value=0;
	Set_Pin_Input(DS18B20_PORT, DS18B20_PIN); // configura como entrada

	for (int i=0;i<8;i++)
	{
		Set_Pin_Output(DS18B20_PORT, DS18B20_PIN); // Configura como saida
		HAL_GPIO_WritePin (GPIOB, GPIO_PIN_8, 0);  // pull the data pin LOW
		delay (2);  // wait for 2 us
		Set_Pin_Input(DS18B20_PORT, DS18B20_PIN); // configura como entrada
		if (HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_8))  // if the pin is HIGH
		{
			value |= 1<<i;  // read = 1
		}
		delay (60);  // wait for 60 us
	}
	return value;
}

void Measure(){
	GPIOB->ODR |= (1 << 4);
	HAL_Delay(2000);
	ADC1_ReadOne(ADC_CHANNEL_9, ADC_SAMPLETIME_13CYCLES_5, &v0);
	ADC1_ReadOne(ADC_CHANNEL_0, ADC_SAMPLETIME_13CYCLES_5, &v1);
	ADC1_ReadOne(ADC_CHANNEL_1, ADC_SAMPLETIME_13CYCLES_5, &v2);

	NTU = ntu_from_adc(v0);
	NTU_regressao = ((NTU/NTU_mul) - NTU_lin);
		  //2.88 e -538

	Presence = DS18B20_Start ();
	DS18B20_Write (0xCC);  // skip ROM
	DS18B20_Write (0x44);  // convert t
	Presence = DS18B20_Start ();
	DS18B20_Write (0xCC);  // skip ROM
	DS18B20_Write (0xBE);  // Read Scratch-pad
	Temp_byte1 = DS18B20_Read();
	Temp_byte2 = DS18B20_Read();
	TEMP = ((Temp_byte2<<8))|Temp_byte1;
	Temperature = (float)TEMP/16.0;

	float v_adc = (float)v1 * ADC_VREF / ADC_FULLSCALE;
	float tempCoef = 1.0f + ALPHA_TEMP * (Temperature - 25.0f);
	float v_comp   = v_adc / tempCoef;
	float v2p = v_comp * v_comp;
	float v3p = v2p * v_comp;
	TDS_ppm = (133.42f * v3p - 255.86f * v2p + 857.39f * v_comp) * TDS_FACTOR_K;
	// --- Condutividade elétrica (µS/cm e mS/cm) ---
	EC_uScm = TDS_ppm / TDS_FACTOR_K;    // inverso do fator -> µS/cm
	EC_mScm = EC_uScm / 1000.0f;         // mS/cm (opcional)
	mScm_regressao = EC_mScm * mScm_mul;
	// Sensor de pH
	v_ph = (float)v2 * (ADC_VREF / (ADC_FULLSCALE * K_DIV)); // Esse K_DIV é a atenuação do operacional
	pH_value   = ((-5.7*v_ph) + 21.81);
	if (pH_value < 0.0f)  pH_value = 0.0f;
	if (pH_value > 14.0f) pH_value = 14.0f;
	GPIOB->ODR &= ~(1 << 4);
}

void TX(void){
	HAL_GPIO_WritePin (MAX485_RX_EN_PORT, MAX485_RX_EN_PIN, 1); // Habilita o TX
	HAL_GPIO_WritePin (MAX485_TX_EN_PORT, MAX485_TX_EN_PIN, 1);
	HAL_Delay(100);
	char msg[128];

	    // Formato: NTU&Temp&mScm&pH\r\n
	int len = snprintf(msg, sizeof(msg),
	                       "#%.2f&%.2f&%.3f&%.2f$",
	                       NTU_regressao, Temperature,
	                       mScm_regressao, pH_value);
	if (len > 0) {
	        HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, HAL_MAX_DELAY);
	}
	HAL_Delay(100);
	HAL_GPIO_WritePin (MAX485_RX_EN_PORT, MAX485_RX_EN_PIN, 0); // Coloca como receptor
	HAL_GPIO_WritePin (MAX485_TX_EN_PORT, MAX485_TX_EN_PIN, 0);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)   // confere se é a USART1
    {
        if (rx_byte == '@') {
            // Ação desejada quando receber '@'
            //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);  // exemplo: inverter PB4
        }

        // Reabilita a recepção do próximo byte
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
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
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  // Mosfet que ativa os sensores está no PB4
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;     // habilita clock de GPIOB
  GPIOB->CRL &= ~(0xF << (4*4));          // limpa configuração do pino 4 (bits 19:16)
  GPIOB->CRL |=  (0x2 << (4*4));          // MODE4 = 10 (2 MHz), CNF4 = 00 (PP)
  GPIOB->ODR &= ~(1 << 4);
  HAL_TIM_Base_Start(&htim2);
  Set_Pin_Output(MAX485_RX_EN_PORT, MAX485_RX_EN_PIN);
  Set_Pin_Output(MAX485_TX_EN_PORT, MAX485_TX_EN_PIN);
  HAL_GPIO_WritePin (MAX485_RX_EN_PORT, MAX485_RX_EN_PIN, 0);// Desativa o TX MAX485
  HAL_GPIO_WritePin (MAX485_TX_EN_PORT, MAX485_TX_EN_PIN, 0);
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //char msg[] = "Hello\r\n";
	  //HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	  if(rx_byte == 0x40){
		  rx_byte = 0;
		  Measure();
		  TX();
		  HAL_Delay(2000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
