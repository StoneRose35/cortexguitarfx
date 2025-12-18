#ifndef _DISPLAY_128_X_64_H_
#define _DISPLAY_128_X_64_H_
#include "stdint.h"

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
uint8_t IsDisplayUpdateOngoing();
#endif