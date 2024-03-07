#ifndef _DS18B20_H_
#define _DS18B20_H_
#include <stdint.h>


uint8_t initDs18b20();
uint8_t writeDs18b20(uint8_t cmd);
uint8_t readDs18b20();
uint8_t resetDs18b20();

uint8_t initTempConversion();
uint8_t readTemp(int16_t*);
uint8_t getTempReadState();
 #endif