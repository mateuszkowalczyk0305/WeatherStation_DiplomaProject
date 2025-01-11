/*
 * AS7331.hpp
 *
 *  Created on: Nov 5, 2024
 *      Author: mateu
 */

#ifndef INC_AS7331_HPP_
#define INC_AS7331_HPP_


#include "main.h"
#include <string>
#include <cmath>

struct UVData {
    double uva;
    double uvb;
    double uvc;
};

struct UVDataString {
	std::string uva;
	std::string uvb;
	std::string uvc;
};

class AS7331
{
	public:
	AS7331(I2C_HandleTypeDef* _hi2c, uint16_t _timeout);

	bool init();

	bool readMeasurements();

	UVData uv_data;

//	UVDataString uv_str_data;
	private:

	I2C_HandleTypeDef* i2c;
	uint16_t timeout;

	const uint16_t AS7331_ADDR = 0x74 << 1;
};




#endif /* INC_AS7331_HPP_ */
