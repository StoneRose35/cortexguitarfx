/*
 * taskManagerUtils.h
 *
 *  Created on: 25.10.2021
 *      Author: philipp
 */

#ifndef INC_TASKMANAGERUTILS_H_
#define INC_TASKMANAGERUTILS_H_

#include <stdint.h>
#include "stringFunctions.h"
#include "system.h"

uint8_t tryToUInt8(char * str,uint8_t* has_errors_ptr);

int16_t tryToInt16(char * str,uint8_t* has_errors_ptr);

uint32_t tryToUInt32(char * str,uint8_t* has_errors_ptr);

uint8_t checkLampRange(uint8_t,uint8_t* has_errors_ptr);

void handleRgb(uint8_t,uint8_t,uint8_t,uint8_t,RGBStream*);

void handleRgbStruct(RGB*,uint8_t,RGBStream*);

/*
 * expands a string list of lamp number into a array of unique lamp numbers
 * */
uint8_t expandLampDescription(char *,uint8_t*);

#endif /* INC_TASKMANAGERUTILS_H_ */
