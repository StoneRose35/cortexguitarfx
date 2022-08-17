#include "imgDisplay.h"
#include "charDisplay.h"
#include "stdlib.h"
#include "spi_sdcard_display.h"

uint8_t displayImage(const ST7735ImageType* img,uint8_t px,uint8_t py)
{
    if ((px + img->columns <=160) && (py + img->rows <= 128))
    {
        setUpdateRange(px,py,img->columns,img->rows);
        sendDisplayCommand(0x2C,img->colorbytes,img->rows*img->columns*2);
        return 0;
    }
    else
    {
        return 1;
    }
}


uint8_t fillSquare(const RGB * clr,uint8_t px,uint8_t py,uint8_t sx,uint8_t sy)
{
    if (sx > 0 && sy > 0)
    {
        uint16_t * squareBfr = (uint16_t*)malloc(sx*sy*2);
        uint16_t clrEncoded = encodeColor(clr);
        for (uint32_t c=0;c<sx*sy;c++)
        {
            *(squareBfr + c) = clrEncoded;
        }
        setUpdateRange(px,py,sx,sy);
        sendDisplayCommand(0x2C,(uint8_t*)squareBfr,sx*sy*2);
        free(squareBfr);
    }
    return 0;
}

void setUpdateRange(uint8_t startX,uint8_t startY,uint8_t sizeX, uint8_t sizeY)
{
    uint8_t cmdData[4];
    cmdData[0] = 0;
    cmdData[1] = startX; 
    cmdData[2] = 0; 
    cmdData[3] = startX+sizeX-1; 
    sendDisplayCommand(0x2A,cmdData,4);
    cmdData[0] = 0;
    cmdData[1] = startY; 
    cmdData[2] = 0; 
    cmdData[3] = startY+sizeY-1; 
    sendDisplayCommand(0x2B,cmdData,4);
}




