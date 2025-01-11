/*
 * AS7331.cpp
 *
 *  Created on: Nov 5, 2024
 *      Author: mateu
 */


#include "AS7331.hpp"

AS7331::AS7331(I2C_HandleTypeDef* _hi2c, uint16_t _timeout)
			: i2c(_hi2c), timeout(_timeout)
{}

bool AS7331::init(){
	// Data buffer and variable for checking communication errors:
	HAL_StatusTypeDef check;
	uint8_t data[2];

	// Set Power Down mode:
	data[0] = 0x00; // OSR
	data[1] = 0x40; // PD=1, DOS=010 - Power Down state
	check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if(check != HAL_OK) return false;

	// Reset Power Down mode and set configuration state:
	data[0] = 0x00; // OSR
	data[1] = 0x02; // PD=0, DOS=010 - Configuration state command
	check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if(check != HAL_OK) return false;

    // Set gain and conversion time
    data[0] = 0x01; // CREG1
    data[1] = 0x0A; // Gain = 2048x, Time = 1024 ms
    check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if(check != HAL_OK) return false;

    // Set measurement mode:
    data[0] = 0x08; // CREG3
    data[1] = 0x00; // MMODE=00 (CONT Mode), SB=0, RDYOD=0, CCLK=00 (1.024 MHz)
    check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if (check != HAL_OK) return false;

    // Set measurement state:
    data[0] = 0x00; // OSR
    data[1] = 0x03; // PD=0, DOS=011 - Measurement state command
    check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if(check != HAL_OK) return false;

    // Set Continuous Measurement Mode:
    data[0] = 0x00;  // Adres OSR
    data[1] = 0x83;  // PD=0, DOS=011, SS=1
    check = HAL_I2C_Master_Transmit(i2c, AS7331_ADDR, data, 2, timeout);
    if(check != HAL_OK) return false;


    HAL_Delay(1000);
    return true;
}


bool AS7331::readMeasurements(){
    HAL_StatusTypeDef check;
    uint8_t data[2];

	// Read UVA (from OUTCONV_A register)
    check = HAL_I2C_Mem_Read(i2c, AS7331_ADDR, 0x02, 1, data, 2, timeout);
	if (check != HAL_OK) return false;
	uint16_t uva_raw = (data[0] << 8) | data[1];
	uv_data.uva = static_cast<double>(uva_raw * 0.16 * pow(10, -3));  // Save UVA in struct (LSB for UVA = 0.16 nW/cm²)

    // Read UVB (OUTCONV_B register at address 0x03)
    check = HAL_I2C_Mem_Read(i2c, AS7331_ADDR, 0x03, 1, data, 2, timeout);
    if (check != HAL_OK) return false;
	uint16_t uvb_raw = (data[0] << 8) | data[1];
	uv_data.uvb = static_cast<double>(uvb_raw * 0.16 * pow(10, -3));  // Save UVA in struct (LSB for UVB = 0.16 nW/cm²)

    // Read UVC (OUTCONV_C register at address 0x04)
    check = HAL_I2C_Mem_Read(i2c, AS7331_ADDR, 0x04, 1, data, 2, timeout);
    if (check != HAL_OK) return false;
	uint16_t uvc_raw = (data[0] << 8) | data[1];
	uv_data.uvc = static_cast<double>(uvc_raw * 0.08 * pow(10, -3));  // Save UVA in struct (LSB for UVC = 0.08 nW/cm²)

	return true;
}

