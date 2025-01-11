/*
 * led.h
 *
 *  Created on: Nov 13, 2024
 *      Author: mateu
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "math.h"


extern TIM_HandleTypeDef htim2;
extern int datasentflag;
extern int flag_data_send;


void Set_LED (int LEDnum, int Red, int Green, int Blue);

void Set_Brightness (int brightness);

void WS2812_Send (void);

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);


#ifdef __cplusplus
}
#endif

#endif /* INC_HCSR04_H_ */

