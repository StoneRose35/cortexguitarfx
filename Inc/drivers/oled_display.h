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
void initOledDisplay();


void OledWriteChar(char chr);
void OledWriteText(const char * str,uint8_t posH,uint8_t posV);
void OledWriteTextLine(const char * str,uint8_t posV);
void OledDisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength);
void OledDisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void OledDisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void OledClearDisplay();
void OledWriteLineAsync(volatile uint8_t * data);
void OledWriteNextLine(void);
void OledwriteFramebufferAsync(uint8_t * fb);


#endif