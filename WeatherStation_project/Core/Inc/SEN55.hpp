/*
 * SEN55.hpp
 *
 *  Created on: Oct 22, 2024
 *      Author: user
 */

#ifndef INC_SEN55_HPP_
#define INC_SEN55_HPP_


// Jeżeli problem nie zniekie po 6h pray spróbuj odzczytać Device Status Register is prawdź czy nie występuje problem z czujnikiem gazu.

#include "main.h"
#include <string>

struct SensorData {
	uint16_t pm1_0;
	uint16_t pm2_5;
	uint16_t pm4_0;
	uint16_t pm10;
	int16_t humidity;     // Wilgotność względna [%RH]
	int16_t temperature;  // Temperatura [°C]
	uint16_t vocIndex;
	uint16_t noxIndex; 	// TUTAJ MAM CHYBA BLAD
};

struct SensorStringData {
	std::string pm1_0;
	std::string pm2_5;
	std::string pm4_0;
	std::string pm10;
	std::string humidity;     // Wilgotność względna [%RH]
	std::string temperature;  // Temperatura [°C]
	std::string vocIndex;
	std::string noxIndex; 	// TUTAJ MAM CHYBA BLAD
};


class SEN55{
public:
	SEN55(I2C_HandleTypeDef* _hi2c, uint16_t timeout);

	// Zwróć nazwę urządzenia:
	std::string getDeviceName();
	// Start pomiarów:
	bool startMeasurement();
	// Stop pomiarów:
	bool stopMeasurement();
	// Odczyt pomiarów:
	SensorData readMeasuremendValues();

	SensorStringData sensor_string_data;

private:
	I2C_HandleTypeDef* i2c;
	uint16_t timeout;

	SensorData sensorData;

	uint8_t calculate_crc(uint8_t *data, uint8_t len);

	const uint16_t SEN55_ADDRESS = 0x69 << 1; // Adres I2C czujnika
};

#endif /* INC_SEN55_HPP_ */
