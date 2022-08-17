#ifndef _CHAR_DISPLAY_H_
#define _CHAR_DISPLAY_H_
#include <stdint.h>


void casetCmd(uint16_t fontPosH,uint8_t fontSizeH);
void rasetCmd(uint16_t fontPosV,uint8_t fontSizeV);

void writeChar(char chr,uint16_t posH,uint16_t posV,uint8_t fontType);
uint8_t writeString(char * str,uint8_t col,uint8_t row);
void writeText(char * str,uint16_t posH,uint16_t posV,uint8_t fontType);


#define BGCOLOR_MSB 0xFF
#define BGCOLOR_LSB 0xF6

#define FGCOLOR_MSB 0x00
#define FGCOLOR_LSB 0x00

#define FONT_TYPE_8X8 0
#define FONT_TYPE_16X16 1


#endif