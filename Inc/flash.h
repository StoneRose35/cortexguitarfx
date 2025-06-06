/*
 * flash.h
 *
 *  Created on: 30.10.2021
 *      Author: philipp
 */

#ifndef FLASH_H_
#define FLASH_H_
#include "stdint.h"




uint8_t unlockFlash();
uint8_t eraseSector();
uint8_t writeFlashWord(uint8_t*data,uint32_t address); // writes 32 bytes to a 32 byte aligned address 

#endif /* FLASH_H_ */
