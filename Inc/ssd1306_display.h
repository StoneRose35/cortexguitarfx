#ifndef _SSD1306_DISPLAY_H_
#define _SSD1036_DISPLAY_H_
#include <stdint.h>

#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/sio.h"


#define SSD1306_DISPLAY_N_PAGES 8
#define SSD1306_DISPLAY_N_COLUMNS 128
void initSsd1306Display();


void ssd1306WriteChar(char chr);
void ssd1306WriteText(const char * str,uint8_t posH,uint8_t posV);
void ssd1306WriteTextLine(const char * str,uint8_t posV);
void ssd1306DisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength);
void ssd1306DisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void ssd1306DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void ssd1306ClearDisplay();
void ssd1306WriteLineAsync(volatile uint8_t * data);
void ssd1306WriteNextLine(void);
void ssd1306writeFramebufferAsync(uint8_t * fb);


#endif