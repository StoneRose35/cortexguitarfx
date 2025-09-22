#ifndef _SSD1306_DISPLAY_H_
#define _SSD1036_DISPLAY_H_
#include <stdint.h>
#include "globalConfig.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/sio.h"



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


void initDisplay();
void setCursor(uint8_t row, uint8_t col);
void ClearDisplay();
void CheckerBoardDisplay();
void DisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength);
void DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img);
void DisplayWriteChar(char chr);
void DisplayWriteText(const char * str,uint8_t posH,uint8_t posV);
void DisplayWriteTextLine(const char * str,uint8_t posV);
void DisplayWriteNextLine(void);
void DisplayWriteFramebufferAsync(uint8_t * fb);

#endif