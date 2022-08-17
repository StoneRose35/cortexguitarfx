#ifndef _IMG_DISPLAY_H_
#define _IMG_DISPLAY_H_

#include <stdint.h>
#include "system.h"

typedef struct ST7735ImageStruct
{
    const uint8_t * colorbytes;
    uint8_t rows;
    uint8_t columns;

} ST7735ImageType;

typedef ST7735ImageType* ST7735Image;

typedef struct 
{
    uint8_t colorbytes[512];
    uint8_t rows;
    uint8_t columns;

} ST7735Font16x16;

inline uint16_t encodeColor(const RGB * clr)
{
    return (((clr->r & 0xF8) | ((clr->g >> 5) & 0x7)) << 0) | 
    ((((clr->g & 0x7) << 5) | ((clr->b & 0xF8) >> 3)) << 8);
}

uint8_t displayImage(const ST7735ImageType* img,uint8_t px,uint8_t py);

uint8_t fillSquare(const RGB * clr,uint8_t px,uint8_t py,uint8_t sx,uint8_t sy);

void setUpdateRange(uint8_t startX,uint8_t startY,uint8_t sizeX, uint8_t sizeY);

#endif
