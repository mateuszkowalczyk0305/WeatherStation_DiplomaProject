/*
 * # AS5600 lib Introduction:
 *
 *	 1. AS5600 lib uses full angle range and calculates values of zero position without using permanent savings in ZPOS, MANG and MPOS registers,
 *	 2. Use setZeroOffset() function to define your zero position dynamically,
 *	 3. getAngleRaw() function returns double raw value in the range <0, 360>.
 *
 * # About AS5600:
 *
 *   The AS5600 is an easy to program magnetic rotary position sensor with a high-resolution 12-bit analog or PWM output. This
 *   contactless system measures the absolute angle of a diametric magnetized on-axis magnet. This AS5600 is designed for contactless
 *   potentiometer applications and its robust design eliminates the influence of any homogenous external stray magnetic fields.
 *   The industry-standard I²C interface supports simple user programming of non-volatile parameters without requiring a dedicated programmer
 *   By default the output represents a range from 0 to 360 degrees.
 *
 *   Datasheet link: https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwjSwYjl_-SHAxXqQVUIHaWTBWwQFnoECA0QAQ&url=https%3A%2F%2Ffiles.seeedstudio.com%2Fwiki%2FGrove-12-bit-Magnetic-Rotary-Position-Sensor-AS5600%2Fres%2FMagnetic%2520Rotary%2520Position%2520Sensor%2520AS5600%2520Datasheet.pdf&usg=AOvVaw3nHa6aZR1ZpE_QRxKPGTok&opi=89978449
*/


#ifndef AS5600_HPP
#define AS5600_HPP

#include <main.h>

// Peripheral:
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

// Libraries:
#include <cmath>

class AS5600
{
public:
	AS5600(I2C_HandleTypeDef* hi2c, uint8_t i2cAddress = 0x36, uint32_t timeout = 100);

// Enum definitions:
	enum PowerMode{
		NOM = 0b00,
		LPM1 = 0b01,
		LPM2 = 0b10,
		LPM3 = 0b11
	};

	enum Hysteresis{
		HYST_OFF = 0b00,
		LSB_1 = 0b01,
		LSBs_2 = 0b10,
		LSBs_3 = 0b11
	};

	enum OutputStage{
		ANALOG_FULL_RANGE = 0b00,
		ANALOG_10_to_90_RANGE = 0b01,
		DIGITAL_PWM = 0b10,
	};

	enum PWM_Frequency{
		Hz115 = 0b00,
		Hz230 = 0b01,
		Hz460 = 0b10,
		Hz920 = 0b11
	};

	enum SlowFilter{
		X16 = 0b00,
		X8 = 0b01,
		X4 = 0b10,
		X2 = 0b11
	};

	enum FastFilterThreshold{
		SLOW_FILTER_ONLY = 0b000,
		LSBs_6 = 0b001,
		LSBs_7 = 0b010,
		LSBs_9 = 0b011,
		LSBs_18 = 0b100,
		LSBs_21 = 0b101,
		LSBs_24 = 0b110,
		LSBs_10 = 0b111
	};

	enum Watchdog{
		OFF = 0,
		ON = 1
	};

	// Status functions:
	bool checkIsMagnetDetected();

	bool checkIsMagnetTooStrong();

	bool checkIsMagnetTooWeak();

	uint8_t getMagnetStatus();

	// Angle functions:
	void setZeroOffset(double offset);

	double getAngleRaw();

	double getAngleDeg();

	int getAngleDegInt();

	double getAngleRad();

	int getAngleRadInt();

	// Configuration functions:
	void setPowerMode(PowerMode power_mode);

	void setHysteresis(Hysteresis hyst_mode);

	void setOutputStage(OutputStage output_stage_mode);

	void setPWM_Frequency(PWM_Frequency pwm_freq_mode);

	void setSlowFilter(SlowFilter slow_filter_mode);

	void setFastFilterThreshold(FastFilterThreshold fast_filter_mode);

	void setWatchdog(Watchdog wtd_mode);

	void setConfiguration(PowerMode power_mode, Hysteresis hyst_mode, OutputStage output_stage_mode, PWM_Frequency pwm_freq_mode,
			SlowFilter slow_filter_mode, FastFilterThreshold fast_filter_mode, Watchdog wtd_mode);

	uint16_t readConfigRegister();

	void resetConfiguration();

private:

	I2C_HandleTypeDef *hi2c;
	uint8_t i2cAddress;
	double zeroOffset;
	uint16_t timeout;

	static constexpr uint8_t RAW_ANGLE = 0x0C;
	static constexpr uint8_t ANGLE = 0x0E;
	static constexpr uint8_t CONF = 0x07;
	static constexpr uint8_t STATUS = 0x0B;

	uint16_t readRegister12(uint8_t reg);
	uint8_t readRegister8(uint8_t reg);

	void writeConfigRegister(uint16_t value);

	uint16_t bitConfiguration(uint8_t bit_count, uint8_t bit_shift, uint8_t value);
};

#endif


