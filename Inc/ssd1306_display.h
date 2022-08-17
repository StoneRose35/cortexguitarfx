#ifndef _SSD1306_DISPLAY_H_
#define _SSD1036_DISPLAY_H_
#include <stdint.h>


void initSsd1306Display();


void ssd1306WriteChar(uint8_t row,uint8_t col,char chr);
void ssd1306WriteText(const char * str,uint8_t posH,uint8_t posV);
void ssd1306DisplayByteArray(uint8_t row,uint8_t col,uint8_t *arr,uint16_t arrayLength);
void ssd1306DisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void ssd1306DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void ssd1306ClearDisplay();



#endif