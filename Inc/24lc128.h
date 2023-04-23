#ifndef _24_LC_128_H_
#define _24_LC_128_H_
#include "stdint.h"
#include "i2c.h"

#define EEPROM_24LC128_ADDRESS 0x50
#define EEPROM_24LC128_PAGE_LENGTH 0x40

void eeprom24lc128ReadArray(uint32_t startAdress,uint16_t len,uint8_t* data);
void eeprom24lc128WriteArray(uint32_t startAdress,uint16_t len, uint8_t* data);

#endif