/*
 * bme280.hpp
 *
 *  Created on: Oct 14, 2024
 *      Author: Mateusz Kowalczyk
 */

#ifndef INC_BME280_HPP_
#define INC_BME280_HPP_


#include "main.h"
#include <string>
#include <cmath>
#include <sstream> // sprawdź czy konieczne
#include <iomanip> // sprawdź czy konieczne

constexpr uint16_t seaLevel = 120;

class BME280
{
	public:
	BME280(I2C_HandleTypeDef* _hi2c, uint16_t _timeout);

	std::string checkSensor();

	void init();
	void reset();

	void setNormalMode();
	void setForcedMode();
	std::string checkMode();

	double getTemp();
	double getHumidity();
	double getAtmPressure();

	private:
	double roundDouble(double number);

	I2C_HandleTypeDef* i2c;
	uint16_t timeout;
	int32_t t_fine;

    // Temperature calibration variables:
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    // Humidity calibration variables:
    int8_t dig_H1;
    int16_t dig_H2;
    int8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;

    // Pressure calibration variables:
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
};


#endif /* INC_BME280_HPP_ */
