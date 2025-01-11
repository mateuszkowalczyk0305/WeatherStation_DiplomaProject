/*
 * SEN55.cpp
 *
 *  Created on: Oct 22, 2024
 *      Author: Mateusz Kowalczyk
 */

#include "SEN55.hpp"

SEN55::SEN55(I2C_HandleTypeDef* _hi2c, uint16_t _timeout)
			:i2c(_hi2c), timeout(_timeout){}

bool SEN55::startMeasurement(){
	// Komenda do startu pomiarów:
	constexpr uint16_t startMeasurementCmd = 0x0021;
	uint8_t cmd[2];

	// Podzielenie komendy na dwa bajty:
	cmd[0] = startMeasurementCmd >> 8;			// Starszy bajt
	cmd[1] = startMeasurementCmd & 0xFF;		// Młodszy bajt (maskowanie w celu zachowania pozostałych danych komendy)

	// Wysłanie komendy startu pomiaru do czujnika:
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(i2c, SEN55_ADDRESS, cmd, sizeof(cmd), timeout);

	// Obsługa błedu wysłania komendy:
	if (status == HAL_OK) return true;
	else return false;
}

bool SEN55::stopMeasurement(){
	// Komenda do zakończenia pomiarów:
    constexpr uint16_t stopMeasurementCmd = 0x0104;
    uint8_t cmd[2];

    // Podzielenie komendy na dwa bajty:
    cmd[0] = stopMeasurementCmd >> 8;   		// Starszy bajt
    cmd[1] = stopMeasurementCmd & 0xFF; 		// Młodszy bajt (maskowanie w celu zachowania pozostałych danych komendy)

    // Wyślij komendę przez I2C
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(i2c, SEN55_ADDRESS, cmd, sizeof(cmd), timeout);

	// Obsługa błedu wysłania komendy:
	if (status == HAL_OK) return true;
	else return false;
}

std::string SEN55::getDeviceName(){

	// Komenda do odczytu nazwy urządzenia:
	constexpr uint16_t readSensorNamecMD = 0xD014;
	uint8_t cmd[2];
	// Podzielenie komendy na dwa bajty:
	cmd[0] = readSensorNamecMD >> 8;  			// Starszy bajt
	cmd[1] = readSensorNamecMD & 0xFF;          // Młodszy bajt (maskowanie w celu zachowania pozostałych danych komendy)

	// Wysłanie komendy do czujnika:
	if (HAL_I2C_Master_Transmit(i2c, SEN55_ADDRESS, cmd, sizeof(cmd), timeout) != HAL_OK) {
		return "I2C Transmit Failed";
	}

	// Delay zgodny z dokumentacją (20 ms):
	HAL_Delay(20);

	// Odbiór odpowiedzi z czujnika (dane[32b] + CRC[16b]):
	uint8_t productNameWithCRC[48] = {0};

	if (HAL_I2C_Master_Receive(i2c, SEN55_ADDRESS, productNameWithCRC, 48, timeout) != HAL_OK) {
		return "I2C Receive Failed";
	}

	// Pętla do przetwarzania i weryfikacji danych:
	std::string name;
	for (int i = 0; i < 32; i += 3) {
		uint8_t charPair[2] = { productNameWithCRC[i], productNameWithCRC[i+1] };
		uint8_t receivedCRC = productNameWithCRC[i+2];
		uint8_t calculatedCRC = calculate_crc(charPair, 2);

		// Obsługa błędu CRC:
		if (receivedCRC != calculatedCRC) {
			return "CRC Check Failed";
		}

		if (charPair[0] != '\0') name += static_cast<char>(charPair[0]);
		if (charPair[1] != '\0') name += static_cast<char>(charPair[1]);

		// Stop w przypadku spotkania znaku końca ciągu znaków:
		if (charPair[0] == '\0' || charPair[1] == '\0') {
			break;
		}
	}

	return name;
}

SensorData SEN55::readMeasuremendValues(){
    constexpr uint16_t readSensorValuesCmd = 0x03C4;
    uint8_t cmd[2];
    uint8_t buffer[24];  // Bufor na 24 bajty danych

    // Podzielenie komendy na dwa bajty
    cmd[0] = readSensorValuesCmd >> 8;   		// Starszy bajt
    cmd[1] = readSensorValuesCmd & 0xFF; 		// Młodszy bajt (maskowanie w celu zachowania pozostałych danych komendy)

    // Wysłanie komendy do czujnika:
    HAL_I2C_Master_Transmit(i2c, SEN55_ADDRESS, cmd, sizeof(cmd), timeout);

    // Opóźnienie zgodne z dokumentacją:
    HAL_Delay(20);

    // Odczytaj dane (24 bajty) danych poamirowych:
    HAL_I2C_Master_Receive(i2c, SEN55_ADDRESS, buffer, sizeof(buffer), timeout);

    // Sprawdzenie sum kontrolnych dla każdego bloku danych:
	if (calculate_crc(&buffer[0], 2) != buffer[2]) {
		// Błąd CRC dla PM1.0
		return {};
	}

	if (calculate_crc(&buffer[3], 2) != buffer[5]) {
		// Błąd CRC dla PM2.5
		return {};
	}

	if (calculate_crc(&buffer[6], 2) != buffer[8]) {
		// Błąd CRC dla PM4.0
		return {};
	}

	if (calculate_crc(&buffer[9], 2) != buffer[11]) {
		// Błąd CRC dla PM10
		return {};
	}

	if (calculate_crc(&buffer[12], 2) != buffer[14]) {
		// Błąd CRC dla wilgotności
		return {};
	}

	if (calculate_crc(&buffer[15], 2) != buffer[17]) {
		// Błąd CRC dla temperatury
		return {};
	}

	if (calculate_crc(&buffer[18], 2) != buffer[20]) {
		// Błąd CRC dla VOC
		return {};
	}

	if (calculate_crc(&buffer[21], 2) != buffer[23]) {
		// Błąd CRC dla NOx
		return {};
	}

    // Konwersja danych i zapis do struktury sensorData:
    sensorData.pm1_0 = ((buffer[0] << 8) | buffer[1]) / 10;						// PM 1.0
    sensorData.pm2_5 = ((buffer[3] << 8) | buffer[4]) / 10;						// PM 2.5
    sensorData.pm4_0 = ((buffer[6] << 8) | buffer[7]) / 10;						// PM 4.0
    sensorData.pm10 = ((buffer[9] << 8) | buffer[10]) / 10;						// PM 10
    sensorData.humidity = ((buffer[12] << 8) | buffer[13]) / 100;  				// Wilgotność względna [%RH]
    sensorData.temperature = ((buffer[15] << 8) | buffer[16]) / 200;  			// Temperatura [°C]
    sensorData.vocIndex = ((buffer[18] << 8) | buffer[19]) / 10;  				// Wskaźnik VOC
    sensorData.noxIndex = ((buffer[21] << 8) | buffer[22]) / 10;  				// Wskaźnik NOx

    // Konwersja danych do typu std::sting:
    sensor_string_data.pm1_0 = std::to_string(sensorData.pm1_0);
    sensor_string_data.pm2_5 = std::to_string(sensorData.pm2_5);
    sensor_string_data.pm4_0 = std::to_string(sensorData.pm4_0);
    sensor_string_data.pm10 = std::to_string(sensorData.pm10);
    sensor_string_data.humidity = std::to_string(sensorData.humidity);
    sensor_string_data.temperature = std::to_string(sensorData.temperature);
    sensor_string_data.vocIndex = std::to_string(sensorData.vocIndex);
    sensor_string_data.noxIndex = std::to_string(sensorData.noxIndex);

    return sensorData;
}

uint8_t SEN55::calculate_crc(uint8_t *data, uint8_t len){
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
