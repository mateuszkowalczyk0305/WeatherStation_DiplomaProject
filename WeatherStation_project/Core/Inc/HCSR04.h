/*
 * HCSR04.h
 *
 *  Created on: Nov 12, 2024
 *      Author: Mateusz Kowalczyk
 */

#ifndef INC_HCSR04_H_
#define INC_HCSR04_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


extern TIM_HandleTypeDef htim1;

extern uint8_t Distance;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

void HCSR04_Read(void);

void HCSR04_delay(uint16_t time);

uint8_t showDistance(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_HCSR04_H_ */
