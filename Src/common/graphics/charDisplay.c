// fonts taken from https://github.com/dhepper/font8x8

// implementation is specific for the ST7735 display driver 

#include "charDisplay.h"
#include "imgDisplay.h"
#include <stdint.h>
#include "spi_sdcard_display.h"
#include "fonts/font8x8_basic.h"
#include "fonts/sm_ascii_16x16.h"

/**
 * @brief write a using a 8x8-Font aligned in fixed character block (a.k.a. "text mode")
 * 
 * @param str the string to display
 * @param col the starting column, from 0 20
 * @param row the starting row, from 0 to 16
 * @return uint8_t always 0
 */
uint8_t writeString(char * str,uint8_t col,uint8_t row)
{
    uint16_t strIndex = 0;
    uint8_t colIndex = col;
    uint8_t rowIndex = row;
    uint8_t writeTab = 0;
    while(*(str+strIndex) != 0)
    {
        // basic support of \r, \n and \t
        if (*(str+strIndex) == '\r')
        {
            colIndex = 0;
        }
        else if (*(str+strIndex) == '\n')
        {
            rowIndex++;
        }
        else if(*(str+strIndex) == '\t')
        {
            colIndex += 4;
            writeTab = 1;
        }
        else
        {
            if (rowIndex<16 && ((colIndex < 20 && writeTab == 0) || (colIndex < 15 && writeTab == 1)))
            {
                if (writeTab == 1)
                {
                    writeChar(*(str+9),colIndex,rowIndex,FONT_TYPE_8X8);
                    colIndex++;
                    writeChar(*(str+9),colIndex,rowIndex,FONT_TYPE_8X8);
                    colIndex++;
                    writeChar(*(str+9),colIndex,rowIndex,FONT_TYPE_8X8);
                    colIndex++;
                    writeChar(*(str+9),colIndex,rowIndex,FONT_TYPE_8X8);
                    colIndex++;
                    writeTab = 0;
                }
                writeChar(*(str+strIndex),colIndex,rowIndex,FONT_TYPE_8X8);
                colIndex++;
            }
            else
            {
                return 1;
            }
        }
        strIndex++;
    }
    return 0;
}

/**
 * @brief puts a single characterat a given position, the position is defined in pixel precision
 * 
 * @param chr the character to write (ascii number)
 * @param posH horizontal position of the top left corner
 * @param posV  vertical position of the top left corner
 */
void writeChar(char chr,uint16_t posH,uint16_t posV,uint8_t fontType)
{
    uint32_t frameBufferCnt = 0;
    uint8_t charFrameBuffer[8*8*2];
    ST7735ImageType img;
    switch (fontType)
    {
    case FONT_TYPE_8X8:
    
        for(uint8_t vcnt=0;vcnt<8;vcnt++)
        {
            for(uint8_t hcnt=0;hcnt<8;hcnt++)
            {
                if (((uint8_t)font8x8_basic[(uint8_t)chr][vcnt] & (1 << hcnt)) == 0)
                {
                    charFrameBuffer[frameBufferCnt++] = BGCOLOR_MSB;
                    charFrameBuffer[frameBufferCnt++] = BGCOLOR_LSB;
                }
                else
                {
                    charFrameBuffer[frameBufferCnt++] = FGCOLOR_MSB;
                    charFrameBuffer[frameBufferCnt++] = FGCOLOR_LSB;
                }
            }
        }
        casetCmd(posH,8);
        rasetCmd(posV,8);
        sendDisplayCommand(0x2C,charFrameBuffer,8*8*2);
        
        break;
    case FONT_TYPE_16X16:
        
        img.columns=16;
        img.rows=16;
        img.colorbytes = (uint8_t*)sm_ascii_16x16[(uint8_t)chr];
        displayImage(&img,posH,posV);
        break;
    default: break;
    }
}

/**
 * @brief writes a single line of text at an arbitrary pixel position
 * 
 * @param str 
 * @param posH 
 * @param posV 
 */
void writeText(char * str,uint16_t posH,uint16_t posV,uint8_t fontType)
{
    uint8_t cnt = 0;
    uint16_t hCurrent=posH;
    while(*(str+cnt) != 0)
    {
        writeChar(*(str+cnt),hCurrent,posV,fontType);
        switch (fontType)
        {
            case FONT_TYPE_8X8:
                hCurrent += 1;
                break;
            case FONT_TYPE_16X16:
                hCurrent += 16;
                break;
            default: break;
        }
        cnt++;
    }
}

void casetCmd(uint16_t fontPosH,uint8_t fontSizeH)
{
    uint8_t cmdData[4];
    cmdData[0] = ((fontPosH << 3) & 0xFF00) >> 8;
    cmdData[1] = ((fontPosH << 3) & 0xFF);
    cmdData[2] = ((((fontPosH + 1) << 3)-1) & 0xFF00) >> 8;
    cmdData[3] = ((((fontPosH + 1) << 3)-1) & 0xFF);    
    sendDisplayCommand(0x2A,cmdData,4);
}

void rasetCmd(uint16_t fontPosV,uint8_t fontSizeV)
{
    uint8_t cmdData[4];
    cmdData[0] = ((fontPosV << 3) & 0xFF00) >> 8;
    cmdData[1] = ((fontPosV << 3) & 0xFF);
    cmdData[2] = ((((fontPosV + 1) << 3)-1) & 0xFF00) >> 8;
    cmdData[3] = ((((fontPosV + 1) << 3)-1) & 0xFF);    
    sendDisplayCommand(0x2B,cmdData,4);
}
