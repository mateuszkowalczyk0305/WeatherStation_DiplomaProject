/*
 * bme280.cpp
 *
 *  Created on: Oct 14, 2024
 *      Author: Mateusz Kowalczyk
 */

#include "bme280.hpp"

BME280::BME280(I2C_HandleTypeDef* _hi2c, uint16_t _timeout)
: i2c(_hi2c), timeout(_timeout), t_fine(0)
{}

std::string BME280::checkSensor(){
	std::string result;
	if (HAL_I2C_IsDeviceReady(i2c, 0x76 << 1, 3, 100) == HAL_OK) {
	    return result = "OK";
	} else {
	    return result = "ERROR";
	}
}

void BME280::init() {
	// Temperature and Pressure:
    uint8_t calib[24];
    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0x88, 1, calib, 24, timeout);

    // Parsing calibration data (temp, pressure):
    dig_T1 = static_cast<uint16_t>((calib[1] << 8) | calib[0]);
    dig_T2 = static_cast<int16_t>((calib[3] << 8) | calib[2]);
    dig_T3 = static_cast<int16_t>((calib[5] << 8) | calib[4]);

    dig_P1 = static_cast<uint16_t>((calib[7] << 8) | calib[6]);
    dig_P2 = static_cast<int16_t>((calib[9] << 8) | calib[8]);
    dig_P3 = static_cast<int16_t>((calib[11] << 8) | calib[10]);
    dig_P4 = static_cast<int16_t>((calib[13] << 8) | calib[12]);
    dig_P5 = static_cast<int16_t>((calib[15] << 8) | calib[14]);
    dig_P6 = static_cast<int16_t>((calib[17] << 8) | calib[16]);
    dig_P7 = static_cast<int16_t>((calib[19] << 8) | calib[18]);
    dig_P8 = static_cast<int16_t>((calib[21] << 8) | calib[20]);
    dig_P9 = static_cast<int16_t>((calib[23] << 8) | calib[22]);

    // Humidity:
    uint8_t calib_h[7];
    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xA1, 1, &calib_h[0], 1, timeout);
    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xE1, 1, &calib_h[1], 7, timeout);

    dig_H1 = static_cast<int8_t>(calib_h[0]); // sdcs
    dig_H2 = static_cast<int16_t>((calib_h[2] << 8) | calib_h[1]);
    dig_H3 = static_cast<int8_t>(calib_h[3]);// dcs
    dig_H4 = static_cast<int16_t>((calib_h[4] << 4) | (calib_h[5] & 0x0F));
    dig_H5 = static_cast<int16_t>((calib_h[6] << 4) | (calib_h[5] >> 4));
    dig_H6 = static_cast<int8_t>(calib_h[7]);
}


void BME280::reset(){
	uint8_t reset_cmd = 0xB6;
	HAL_I2C_Mem_Write(i2c, 0x76 << 1, 0xE0, 1, &reset_cmd, 1, timeout);
}

std::string BME280::checkMode(){
    uint8_t ctrl_meas_reg;

    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xF4, 1, &ctrl_meas_reg, 1, timeout);

    uint8_t mode = ctrl_meas_reg & 0x03;

    std::string result;
    if (mode == 0x00) {
        result = "sleep";
    } else if (mode == 0x01 || mode == 0x02) {
    	result = "forced";
    } else if (mode == 0x03) {
    	result = "normal";
    } else {
    	result = "error";;
    }
    return result;
}

void BME280::setNormalMode(){
    uint8_t ctrl_meas_reg = (1 << 5) | (1 << 2) | 3;
    uint8_t config_reg = (5 << 5); // t_standby = 1000 ms, IIR filter off

    HAL_I2C_Mem_Write(i2c, 0x76 << 1, 0xF4, 1, &ctrl_meas_reg, 1, timeout);
    HAL_I2C_Mem_Write(i2c, 0x76 << 1, 0xF5, 1, &config_reg, 1, timeout);
}

void BME280::setForcedMode(){
	uint8_t ctrl_meas_reg;
	ctrl_meas_reg = (1 << 5) | (1 <<2) | (1);
	HAL_I2C_Mem_Write(i2c, 0x76 << 1, 0xF4, 1, &ctrl_meas_reg, 1, timeout);
}

double BME280::getTemp(){
	uint8_t buffer[3];
    int32_t adc_T;
    int32_t var1, var2;
    int32_t temp;
//    int32_t t_fine;

    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xFA, 1, static_cast<uint8_t*>(buffer), 3, timeout);

    // Change result to 12-bit value:
    adc_T = static_cast<int32_t>(
    		(static_cast<uint32_t>(buffer[0]) << 12) |
			(static_cast<uint32_t>(buffer[1]) << 4) |
			(static_cast<uint32_t>(buffer[2]) >> 4));

    // Calculating temperature:
    var1 = static_cast<int32_t>((adc_T >> 3) - (static_cast<int32_t>(dig_T1) << 1));
    var1 = (var1 * static_cast<int32_t>(dig_T2)) >> 11;

    var2 = static_cast<int32_t>(((adc_T >> 4) - static_cast<int32_t>(dig_T1)) *
            ((adc_T >> 4) - static_cast<int32_t>(dig_T1)) >> 12);
    var2 = (var2 * static_cast<int32_t>(dig_T3)) >> 14;

    // Converting into celsius degrees:
    t_fine = var1 + var2;
    temp = (t_fine * 5 + 128) >> 8;
    double temperature = temp / 100.0;

    return roundDouble(temperature);
}

double BME280::getHumidity() {

	getTemp();
    uint8_t buffer[2];
    int32_t adc_H;


    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xFD, 1, buffer, 2, timeout);

    // Compilation of Humidity data:
    adc_H = (static_cast<int32_t>(buffer[0]) << 8) | buffer[1];

    // Humidity conversion using calibration data:
    int32_t v_x1_u32r = (t_fine - (int32_t)76800);
    v_x1_u32r = (((((adc_H << 14) - ((int32_t)dig_H4 << 20) - ((int32_t)dig_H5 * v_x1_u32r)) + (int32_t)16384) >> 15) *
                  (((((((v_x1_u32r * (int32_t)dig_H6) >> 10) * (((v_x1_u32r * (int32_t)dig_H3) >> 11) + (int32_t)32768)) >> 10) + (int32_t)2097152) * (int32_t)dig_H2 + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * (int32_t)dig_H1) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    double humidity = (v_x1_u32r >> 12) / 1024.0;

    return roundDouble(humidity);
}

double BME280::getAtmPressure() {
	getTemp();
    uint8_t buffer[3];
    int32_t adc_P;

    HAL_I2C_Mem_Read(i2c, 0x76 << 1, 0xF7, 1, buffer, 3, timeout);

    // Compilation of pressure data:
    adc_P = (static_cast<int32_t>(buffer[0]) << 12) | (static_cast<int32_t>(buffer[1]) << 4) | (buffer[2] >> 4);

    // Pressure conversion using calibration data:
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

//    if (var1 == 0) {
//        return 0;
//    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

//    double pressure = static_cast<double>(p) / 256.0;

//    return roundDouble(pressure);
    double correct_p = (p/256.0) * pow(1 - (seaLevel/44330.0), -5.255);
    return correct_p;
}



double BME280::roundDouble(double number){
	return std::round(number * 100.0) / 100.0;
}
