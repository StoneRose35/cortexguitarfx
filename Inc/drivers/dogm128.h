#ifndef _DOGM128_DISPLAY_H_
#define _DOGM128_DISPLAY_H_
#include "stdint.h"
#define DOGM128_N_PAGES 8
#define DOGM128_DISPLAY_N_COLUMNS 128
#define HORIZONTAL_OFFSET 0 // set to 4 if display is flipped

void dogm128SendCommand(uint8_t cmd);
void dogm128SendData(const uint8_t*data,uint8_t l);
#endif