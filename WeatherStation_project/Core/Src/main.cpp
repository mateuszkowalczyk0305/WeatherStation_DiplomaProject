/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SEN55.hpp"
#include "bme280.hpp"
#include "AS7331.hpp"
#include <ssd1306.hpp>
#include "ssd1306_fonts.h"
#include "HCSR04.h"
#include "led.h"
// Biblioteki do konwersji ciągów znakowych:
#include <cstring>
#include <string>
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

// Enum zstanów urządzenia::
enum State{
	State_IDLE,
	State_Start,
	State_SEN55,
	State_AS7331,
	State_BME280,
	State_AS5600,
};


// Zmienne do określania poziomu morza:
State currentState = State_IDLE;
uint8_t startStateDistanceCmMax = 5;
uint8_t startStateDistanceCmMin = 2;
uint8_t goToIDLEDistance = 50;

volatile uint64_t systick = 0;


// Funkcje pomocnicze do osbsługi ciągów znakowych/liczb za pomoca biblioteki do kontrolera OLED ssd1306:
char* stringToCharArray(const std::string& text) {
    char* charArray = new char[text.size() + 1];
    std::strcpy(charArray, text.c_str());
    return charArray;
}

std::string doubleToString(double value) {
    char buffer[32];
    sprintf(buffer, "%.2f", value);
    return std::string(buffer);
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // Inicjalizacja:

  // OLED:
  ssd1306_Init();
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen();

  // Włączenie systicka:
  SysTick_Config(SystemCoreClock / 1000);

  // AS7331:
  AS7331 as7331(&hi2c3, 100);
  as7331.init();

  // BME280:
  BME280 bme280(&hi2c2, 100);
  bme280.setNormalMode();
  bme280.init();

  // SEN55
  SEN55 sen55(&hi2c1, 1000);
//  sen55.getDeviceName();
  sen55.startMeasurement();
  HAL_Delay(1000);

  // HC-SR04 timer:
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);

  // Ustaw domyślny stan maszyny stanów:
  currentState = State_IDLE;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  // Maszyna stanów stacji pogodowej:
	  switch(currentState)
	  {
	  case State_IDLE:
		  HCSR04_Read();
		  if (systick % 100 == 0 || systick < 100){

			  // Wyświetlenie napisu ekranu uśpienia:
			  ssd1306_Fill(Black);
			  ssd1306_SetCursor(19,28);
			  char* text1 = stringToCharArray("STACJA POGODOWA");
			  ssd1306_WriteString(text1, Font_6x8, White);
			  ssd1306_UpdateScreen();
			  delete[] text1; // Zwolnienie pamięci po użyciu

			  // Sprawdzenie czujnika odległości:
			if (Distance <= startStateDistanceCmMax && Distance >= startStateDistanceCmMin){
			  currentState = State_Start;
			  systick = 0;
			}}
		  break;
	  case State_Start:
		  if (systick % 1000 == 0 || systick < 100){
			  // Informacje wyświetlane:
			  ssd1306_Fill(Black);
			  ssd1306_SetCursor(16, 0);
			  char* text2 = stringToCharArray("ODCZYTAJ PARAMETR:");
			  ssd1306_WriteString(text2, Font_6x8, White);
			  delete[] text2;

			  ssd1306_SetCursor(0, 10);
			  char* text3 = stringToCharArray("1. PODST. PARAMETRY");
			  ssd1306_WriteString(text3, Font_6x8, White);
			  delete[] text3;

			  ssd1306_SetCursor(0, 20);
			  char* text4 = stringToCharArray("2. JAKOSC POWIETRZA");
			  ssd1306_WriteString(text4, Font_6x8, White);
			  delete[] text4;

			  ssd1306_SetCursor(0, 30);
			  char* text5 = stringToCharArray("3. PROMIENIOWANIE UV");
			  ssd1306_WriteString(text5, Font_6x8, White);
			  delete[] text5;

			  ssd1306_SetCursor(0, 40);
			  char* text6 = stringToCharArray("4. KIERUNEK WIATRU");
			  ssd1306_WriteString(text6, Font_6x8, White);
			  delete[] text6;

			  ssd1306_UpdateScreen();

			  bme280.init();
			  bme280.setNormalMode();
		  }
		  break;
	  case State_SEN55:
		  if (systick % 1000 == 0 || systick < 100){
			  if (systick < 5000)
			  {
				  // Informacje wyświetlane:
				  ssd1306_Fill(Black);
				  sen55.readMeasuremendValues();

				  ssd1306_SetCursor(5,5);
				  char* text1 = stringToCharArray("JAKOSC POWIETRZA 1/2:");
				  ssd1306_WriteString(text1, Font_6x8, White);
				  delete[] text1;

				  ssd1306_SetCursor(5,20);
				  char* text2 = stringToCharArray("PM 1.0: " + sen55.sensor_string_data.pm1_0 + " ug/m3");
				  ssd1306_WriteString(text2, Font_6x8, White);
				  delete[] text2;

				  ssd1306_SetCursor(5,35);
				  char* text3 = stringToCharArray("PM 2.5: " + sen55.sensor_string_data.pm2_5 + " ug/m3");
				  ssd1306_WriteString(text3, Font_6x8, White);
				  delete[] text3;

				  ssd1306_SetCursor(5, 50);
				  char* text4 = stringToCharArray("PM 4.0: " + sen55.sensor_string_data.pm4_0 + " ug/m3");
				  ssd1306_WriteString(text4, Font_6x8, White);
				  delete[] text4;
				  ssd1306_UpdateScreen();
			  } else {
				  // Informacje wyświetlane:
				  ssd1306_Fill(Black);

				  ssd1306_SetCursor(5,5);
				  char* text5 = stringToCharArray("JAKOSC POWIETRZA 2/2:");
				  ssd1306_WriteString(text5, Font_6x8, White);
				  delete[] text5;

				  ssd1306_SetCursor(5,20);
				  char* text6 = stringToCharArray("PM 10: " + sen55.sensor_string_data.pm10 + " ug/m3");
				  ssd1306_WriteString(text6, Font_6x8, White);
				  delete[] text6;

				  ssd1306_SetCursor(5,35);
				  char* text7 = stringToCharArray("VOC:   " + sen55.sensor_string_data.vocIndex + " [0-500]");
				  ssd1306_WriteString(text7, Font_6x8, White);
				  delete[] text7;

				  ssd1306_SetCursor(5, 50);
				  char* text8 = stringToCharArray("NOX:   " + sen55.sensor_string_data.noxIndex + " [0-500]");
				  ssd1306_WriteString(text8, Font_6x8, White);
				  delete[] text8;

				  ssd1306_UpdateScreen();
			  }
		  }

		  break;
	  case State_AS7331:
		  if (systick % 1000 == 0 || systick < 100){
			  ssd1306_Fill(Black);
			  // Odczyt parametrów:
			  as7331.readMeasurements();

			  // Informacje wyświetlane:
			  ssd1306_SetCursor(5,5);
			  char* text1 = stringToCharArray("Promieniowanie UV:");
			  ssd1306_WriteString(text1, Font_6x8, White);
			  delete[] text1;

			  ssd1306_SetCursor(5,20);
			  char* text2 = stringToCharArray("UVA: " + doubleToString(as7331.uv_data.uva) + " uW/cm2");
			  ssd1306_WriteString(text2, Font_6x8, White);
			  delete[] text2;

			  ssd1306_SetCursor(5,35);
			  char* text3 = stringToCharArray("UVB: " + doubleToString(as7331.uv_data.uvb) + " uW/cm2");
			  ssd1306_WriteString(text3, Font_6x8, White);
			  delete[] text3;

			  ssd1306_SetCursor(5, 50);
			  char* text4 = stringToCharArray("UVC: " + doubleToString(as7331.uv_data.uvc) + " uW/cm2");
			  ssd1306_WriteString(text4, Font_6x8, White);
			  delete[] text4;

			  ssd1306_UpdateScreen();
		  }
		  break;
	  case State_AS5600:
		  if (systick % 1000 == 0 || systick < 100){
			  ssd1306_Fill(Black);

			  ssd1306_SetCursor(5,5);
			  char* text1 = stringToCharArray("Kierunek wiatru:");
			  ssd1306_WriteString(text1, Font_6x8, White);
			  delete[] text1;

			  ssd1306_SetCursor(5,35);
			  char* text3 = stringToCharArray("brak odczytu...");
			  ssd1306_WriteString(text3, Font_6x8, White);
			  delete[] text3;

			  ssd1306_UpdateScreen();
		  }
		  break;
	  case State_BME280:
		  if (systick % 1000 == 0 || systick < 100){
			  ssd1306_Fill(Black);

			  double pressure = bme280.getAtmPressure();
			  sen55.readMeasuremendValues();

			  ssd1306_SetCursor(5,5);
			  char* text1 = stringToCharArray("Podstawowe Parametry:");
			  ssd1306_WriteString(text1, Font_6x8, White);
			  delete[] text1;

			  ssd1306_SetCursor(5,20);
			  char* text2 = stringToCharArray("Temp:  " + sen55.sensor_string_data.temperature + " C deg");
			  ssd1306_WriteString(text2, Font_6x8, White);
			  delete[] text2;

			  ssd1306_SetCursor(5,35);
			  char* text3 = stringToCharArray("Hum:   " + sen55.sensor_string_data.humidity + " %");
			  ssd1306_WriteString(text3, Font_6x8, White);
			  delete[] text3;

			  ssd1306_SetCursor(5, 50);
			  char* text4 = stringToCharArray("Press: " + doubleToString(static_cast<double>(pressure/100.0)) + " hPa");
			  ssd1306_WriteString(text4, Font_6x8, White);
			  delete[] text4;

			  ssd1306_UpdateScreen();
		  }
		  break;
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

/* USER CODE BEGIN 4 */

// Przerwanie co 1 ms systick:
void HAL_SYSTICK_Callback(void)
{
//	if (currentState != State_IDLE) ++systick;
	++systick;

	if (systick >= 60000) systick = 0; // Jeżeli dłużej niż jedna minuta

	if (systick > 10000){
		currentState = State_IDLE; // Jeżeli dłużej niż 10 sekund
		systick = 0;
	}
}

// Przerwania od przycisków:
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	systick = 0;
    if (GPIO_Pin == BTN1_Pin) {
        // Obsługa dla przycisku 1 - podstawowe parametry:
        currentState = State_BME280;
    }
    else if (GPIO_Pin == BTN2_Pin) {
        // Obsługa dla przycisku 2 - jakosc powierrza:
        currentState = State_SEN55;
    }
    else if (GPIO_Pin == BTN3_Pin) {
        // Obsługa dla przycisku 3 - promieniowanie uv:
        currentState = State_AS7331;
    }
    else if (GPIO_Pin == BTN4_Pin) {
        // Obsługa dla przycisku 4 - kierunek wiatru:
        currentState = State_AS5600;
    }
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
