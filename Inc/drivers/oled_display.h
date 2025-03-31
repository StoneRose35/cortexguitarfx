#ifndef _SSD1306_DISPLAY_H_
#define _SSD1036_DISPLAY_H_
#include <stdint.h>
#include "globalConfig.h"




#ifdef SH1107
#define SSD1306_DISPLAY_N_PAGES 16
#define SSD1306_DISPLAY_N_COLUMNS 64
#define HORIZONTAL_OFFSET 32
#endif
#ifdef SH1106
#define SSD1306_DISPLAY_N_PAGES 8
#define SSD1306_DISPLAY_N_COLUMNS 128
#define HORIZONTAL_OFFSET 2
#endif

void OledDisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);



#endif