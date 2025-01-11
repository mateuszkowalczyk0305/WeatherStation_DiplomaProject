/*
 * led.c
 *
 *  Created on: Nov 13, 2024
 *      Author: mateu
 */
#include "led.h"

//#define MAX_LED 16
//#define USE_BRIGHTNESS 1
//
//
//uint8_t LED_Data[MAX_LED][4];
//uint8_t LED_Mod[MAX_LED][4];  // for brightness
//
//
//void Set_LED (int LEDnum, int Red, int Green, int Blue)
//{
//	LED_Data[LEDnum][0] = LEDnum;
//	LED_Data[LEDnum][1] = Green;
//	LED_Data[LEDnum][2] = Red;
//	LED_Data[LEDnum][3] = Blue;
//}
//
//#define PI 3.14159265
//
//void Set_Brightness (int brightness)  // 0-45
//{
//#if USE_BRIGHTNESS
//
//	if (brightness > 45) brightness = 45;
//	for (int i=0; i<MAX_LED; i++)
//	{
//		LED_Mod[i][0] = LED_Data[i][0];
//		for (int j=1; j<4; j++)
//		{
//			float angle = 90-brightness;  // in degrees
//			angle = angle*PI / 180;  // in rad
//			LED_Mod[i][j] = (LED_Data[i][j])/(tan(angle));
//		}
//	}
//
//#endif
//}
//
//uint16_t pwmData[(24*MAX_LED)+50];
//
//void WS2812_Send (void)
//{
//	uint32_t indx=0;
//	uint32_t color;
//
//
//	for (int i= 0; i<MAX_LED; i++)
//	{
//#if USE_BRIGHTNESS
//		color = ((LED_Mod[i][1]<<16) | (LED_Mod[i][2]<<8) | (LED_Mod[i][3]));
//#else
//		color = ((LED_Data[i][1]<<16) | (LED_Data[i][2]<<8) | (LED_Data[i][3]));
//#endif
//
//		for (int i=23; i>=0; i--)
//		{
//			if (color&(1<<i))
//			{
//				pwmData[indx] = 66;  // 2/3
//			}
//
//			else pwmData[indx] = 33;  // 1/3
//
//			indx++;
//		}
//
//	}
//
//	for (int i=0; i<50; i++)
//	{
//		pwmData[indx] = 0;
//		indx++;
//	}
//
//	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)pwmData, indx);
//	while (!datasentflag){};
//	datasentflag = 0;
//}
//
//void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
//	if (htim->Instance == TIM2){
//		HAL_TIMEx_PWMN_Stop_DMA(&htim2, TIM_CHANNEL_1);
//		datasentflag = 1;
//	}
//}
//void Reset_Leds(void)
//{
//	for(int i = 0; i < MAX_LED; i++){
//		Set_LED(i, 0, 0, 0);
//		Set_Brightness(45);
//		WS2812_Send();
//	}
//
//}

#define MAX_LED 12
#define USE_BRIGHTNESS 1

uint8_t LED_Data[MAX_LED][4];
uint8_t LED_Mod[MAX_LED][4]; // for brightness

int flag_data_send = 0;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIMEx_PWMN_Stop_DMA(&htim2, TIM_CHANNEL_1);
	flag_data_send = 1;
}

void Set_LED (int LEDnum, int Red, int Green, int Blue) // RGB
{
	LED_Data[LEDnum][0] = LEDnum;
	LED_Data[LEDnum][1] = Green;
	LED_Data[LEDnum][2] = Red;
	LED_Data[LEDnum][3] = Blue;
}

#define PI 3.14159265

void Set_Brightness (int brightness) // [0 - 45]
{
#if USE_BRIGHTNESS

	if(brightness > 45) brightness = 45;
	for (int i = 0; i<MAX_LED; i++)
	{
		LED_Mod[i][0] = LED_Data[i][0];
		for (int j = 1; j < 4; j++)
		{
			float angle = 90 - brightness; // in degrees
			angle = angle*PI / 180; // radians
			LED_Mod[i][j] = (LED_Data[i][j])/(tan(angle));
		}
	}

#endif
}

uint16_t pwmData[(24*MAX_LED)+50]; // + 50 to reset

void WS2812_Send(void)
{
	uint32_t indx = 0;
	uint32_t color;

	for (int i = 0; i < MAX_LED; i++)
	{
		color = ((LED_Mod[i][1]<<16) | (LED_Mod[i][2]<<8) |(LED_Mod[i][3]));

		for (int i = 23; i >= 0; i--)
		{
			if(color&(1<<i))
			{
				pwmData[indx] = 67; // 2/3 of 100
			}
			else pwmData[indx] = 33; // 1/3 of 100

			indx ++;
		}
	}
	for (int i = 0; i < 50; i++)
	{
		pwmData[indx] = 0;
		indx++;
	}

	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)pwmData, indx);
	while (!flag_data_send){};
	flag_data_send = 0;
}
