//#ifdef __I2C_H__
// Header file:
#include "as5600.hpp"

AS5600::AS5600(I2C_HandleTypeDef* hi2c, uint8_t i2cAddress, uint32_t timeout)
	: hi2c(hi2c), i2cAddress(i2cAddress), zeroOffset(0), timeout(timeout) {}

// Status functions:
bool AS5600::checkIsMagnetDetected()
{
	uint8_t raw_status = readRegister8(STATUS);
	uint8_t mask = 0x20;

	uint8_t MD_value = raw_status & mask;

	if (MD_value != 0)
	{
		return 1;
	}else{
		return 0;
	}
}

bool AS5600::checkIsMagnetTooStrong()
{
	uint8_t raw_status = readRegister8(STATUS);
	uint8_t mask = 0x8;

	uint8_t MH_value = raw_status & mask;

	if (MH_value != 0)
	{
		return 1;
	}else{
		return 0;
	}
	//return (MH_value)? 1:0; // rozszerz
}

bool AS5600::checkIsMagnetTooWeak()
{
	uint8_t raw_status = readRegister8(STATUS);
	uint8_t mask = 0x10;

	uint8_t ML_value = raw_status & mask;

	if (ML_value != 0)
	{
		return 1;
	}else{
		return 0;
	}
}

uint8_t AS5600::getMagnetStatus()
{
	uint8_t raw_status = readRegister8(STATUS);
	uint8_t mask = 0x38;

	uint8_t status_value = mask & raw_status;

	return status_value;
}

// Angle functions:
void AS5600::setZeroOffset(double offset)
{
	zeroOffset = offset;
}

double AS5600::getAngleRaw()
{
	uint16_t register12b_resolution = pow(2,12);
	uint16_t full_angle_range = 360;

	uint16_t rawAngle = readRegister12(ANGLE);

	// Range <0 ; 360>
	double raw_angle_deg = (double)rawAngle * full_angle_range/register12b_resolution;
	return raw_angle_deg;
}

double AS5600::getAngleDeg()
{
	uint16_t full_angle_range = 360;

	double not_centered_value = AS5600::getAngleRaw();
	double centered_value = not_centered_value - zeroOffset;

	if ((centered_value < -180) && (centered_value > -zeroOffset))
	{
		centered_value += full_angle_range;
	}

	return centered_value;
}

int AS5600::getAngleDegInt()
{
	double angle_double_value = AS5600::getAngleDeg();
	int angle_value = static_cast<int>(angle_double_value);

	return angle_value;
}

double AS5600::getAngleRad()
{
	double pi = 3.14159265359;
	uint8_t pi_deg = 180;

	double deg_value = AS5600::getAngleDeg();
	double rad_value = deg_value * (pi / pi_deg);

	return rad_value;
}

int AS5600::getAngleRadInt()
{
	double angle_double_value = AS5600::getAngleRad();
	int angle_value = static_cast<int>(angle_double_value);

	return angle_value;
}

// Configuration:
void AS5600::setPowerMode(PowerMode power_mode)
{
	uint16_t conf_reg = bitConfiguration(2, 0, power_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setHysteresis(Hysteresis hyst_mode)
{
	uint16_t conf_reg = bitConfiguration(2, 2, hyst_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setOutputStage(OutputStage output_stage_mode)
{
	uint16_t conf_reg = bitConfiguration(2, 4, output_stage_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setPWM_Frequency(PWM_Frequency pwm_freq_mode)
{
	uint16_t conf_reg = bitConfiguration(2, 6, pwm_freq_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setSlowFilter(SlowFilter slow_filter_mode)
{
	uint16_t conf_reg = bitConfiguration(2, 8, slow_filter_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setFastFilterThreshold(FastFilterThreshold fast_filter_mode)
{
	uint16_t conf_reg = bitConfiguration(3, 10, fast_filter_mode);
	writeConfigRegister(conf_reg);
}

void AS5600::setWatchdog(Watchdog wtd_mode)
{
	uint16_t conf_reg = bitConfiguration(1, 13, wtd_mode);
	writeConfigRegister(conf_reg);
}

uint16_t AS5600::bitConfiguration(uint8_t bit_count, uint8_t bit_shift, uint8_t value)
{

	if (bit_count > 3)
	{
		return 0;
	}else{
	    uint16_t conf_reg = readConfigRegister();
	    uint16_t mask = ((1 << bit_count) - 1) << bit_shift; // np. dla hyst: (1 << 2) -> 0b0100, ((0b0100)- 1) -> 0b0011

	    // Clear config register bits:
	    conf_reg &= ~mask;
	    // Set bits in config register:
	    conf_reg |= (value << bit_shift) & mask;

	    return conf_reg;
	}
}

uint16_t AS5600::readConfigRegister()
{
	// I2C register reading:
	uint8_t buffer[2];
	HAL_I2C_Mem_Read(hi2c, i2cAddress << 1, CONF, 1, buffer, 2, timeout);

	// masking:
	uint16_t raw_value16b = (buffer[0] << 8) | buffer[1];
	uint16_t raw_value14b = raw_value16b & 0x3fff;
	return raw_value14b;
}

void AS5600::resetConfiguration()
{
	AS5600::writeConfigRegister(0);
}

void AS5600::setConfiguration(PowerMode power_mode, Hysteresis hyst_mode, OutputStage output_stage_mode, PWM_Frequency pwm_freq_mode,
		SlowFilter slow_filter_mode, FastFilterThreshold fast_filter_mode, Watchdog wtd_mode)
{
	AS5600::setPowerMode(power_mode);
	AS5600::setHysteresis(hyst_mode);
	AS5600::setOutputStage(output_stage_mode);
	AS5600::setPWM_Frequency(pwm_freq_mode);
	AS5600::setSlowFilter(slow_filter_mode);
	AS5600::setFastFilterThreshold(fast_filter_mode);
	AS5600::setWatchdog(wtd_mode);
}

// Private functions:
uint16_t AS5600::readRegister12(uint8_t reg)
{
	// I2C register reading:
	uint8_t buffer[2];
	HAL_I2C_Mem_Read(hi2c, i2cAddress << 1, reg, 1, buffer, 2, timeout);

	// masking:
	uint16_t raw_value16b = (buffer[0] << 8) | buffer[1];
	uint16_t raw_value12b = raw_value16b & 0xfff;
	return raw_value12b;
}

uint8_t AS5600::readRegister8(uint8_t reg)
{
	uint8_t buffer;
	HAL_I2C_Mem_Read(hi2c, i2cAddress << 1, reg, 1, &buffer, 1, timeout);

	return buffer;
}

void AS5600::writeConfigRegister(uint16_t value)
{
	uint16_t mask = 0x3FFF;
	uint16_t masked_value = value & mask;

	// Data buffor:
	uint8_t buffor[2];
	buffor[0] = masked_value >> 8;
	buffor[1] = masked_value;

	HAL_I2C_Mem_Write(hi2c, i2cAddress << 1, CONF, 1, buffor, 2, timeout);
}
//#endif
