/**
 * @file ssd1306_display.c
 * @author philipp fuerholz
 * @brief driver for a 128*64 oled display driven by a ssd1306 interface using 4-pin spi
 * @version 0.1
 * @date 2022-03-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "ssd1306_display.h"
#include "systick.h"
#include "fonts/oled_font_5x7.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"

void initSsd1306Display()
{
    // get spi out of reset
    *RESETS |= (1 << RESETS_RESET_SPI0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_SPI0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_SPI0_LSB)) == 0);

    // wire up the spi
    *SSD1306_MOSI_PIN_CNTR = 1;
    *SSD1306_SCK_PIN_CNTR = 1;
    *SSD1306_CS_DISPLAY_PIN_CNTR = 1;

    *GPIO_OE |= (1 << SSD1306_DISPLAY_RESET);
    *GPIO_OE |= (1 << SSD1306_DISPLAY_CD); 
    *SSD1306_DISPLAY_CD_PIN_CNTR = 5;
    *SSD1306_DISPLAY_RESET_PIN_CNTR = 5;

    // configure control register 0: 8-bit data, 5 MHz Clock
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (SSD1306_SCK_DISPLAY_SLOW << SPI_SSPCR0_SCR_LSB);
    // configure clock divider
    *SSPCPSR = 2;
    // configure control register 1: enable by setting synchronous operation
    *SSPCR1 = 1 << SPI_SSPCR1_SSE_LSB;



    // reset high
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_RESET);
    waitSysticks(1);
    // reset low
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_RESET);
    waitSysticks(1);
    // reset high
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_RESET);
    waitSysticks(1);

    // manually set display offset and  startline since these two values turned out to be wrong after reset
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    *SSPDR = 0x40; // set startline 0
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    // set displayoffset 0
    *SSPDR = 0xD3;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 


    // send display on command
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    *SSPDR = 0xAF;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    waitSysticks(11);
}

/**
 * @brief Set the Cursor object
 * 
 * @param row goes from 0 to 7, defines the page to write
 * @param col columns, goes from 0 to 127
 */
void setCursor(uint8_t row, uint8_t col)
{
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    // set column, low nibble
    *SSPDR = ((col+2) & 0x0F);
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    // set column, high nibble
    *SSPDR = 0x10 | ((col+2) >> 4);
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // set row / page
    *SSPDR = 0xB0 | row;
     while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );

}

void ssd1306ClearDisplay()
{
    for(uint8_t r=0;r<8;r++)
    {
        for(uint8_t c=0;c<128;c++)
        {
            setCursor(r,c);
            *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // switch to data
            *SSPDR = 0x0;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        }
    }   
}

/**
 * @brief fill a full or parts of a row with bytes
 * 
 * @param row the row from 0 to 7
 * @param col starting column from 0 to 127
 * @param arr the data array (lsb is on top)
 * @param arrayLength the length of the array
 */
void ssd1306DisplayByteArray(uint8_t row,uint8_t col,uint8_t *arr,uint16_t arrayLength)
{
    setCursor(row,col);
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // switch to data
    for (uint16_t c=0;c<arrayLength;c++)
    {
        *SSPDR = *(arr + c);
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }
}

/**
 * @brief displays an image defines as a row-first array
 * 
 * @param px x value of the top left position (0 to 127)
 * @param py y values of the top left position (0 to 7)
 * @param sx x size of the image
 * @param sy y size of the image in pages (8 bit)
 * @param img the image data, the number of bytes must be sx*sy
 */
void ssd1306DisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    //setCursor(py,px);
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);

    // set vertical addressing mode
    *SSPDR = 0x20;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *SSPDR = 0x02;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );

    // set column address
    *SSPDR = 0x21;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *SSPDR =px;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );    
    *SSPDR =px+sx;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );  

    // set page address
    *SSPDR = 0x22;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *SSPDR =py;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );    
    *SSPDR =py+sy;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );        

    uint16_t c=0;
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD);
    while(c<sx*sy)
    {
        *SSPDR = *(img + c);
        c++;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }
}

/**
 * @brief displays a byte array containing y-x ordered image data using standard addressing mode
 * y-x ordered image is
 * 
 * img[0]       img[sy]             .  .        img[(sx-1)*sy]
 * img[1]       img[sy+1]                                .
 *   .             .                                     .
 *   .             .                                     .
 * img[sy-1]    img[2*sy-1]         .  .        img[sx*sy-1]
 * @param px x value of the top left position (0 to 127)
 * @param py y values of the top left position (0 to 7)
 * @param sx x size of the image
 * @param sy y size of the image in pages (8 bit)
 * @param img the image data, the number of bytes must be sx*sy
 */
void ssd1306DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    uint16_t index;
    for(uint8_t cc=0;cc<sy;cc++)
    {
        setCursor(py+cc,px);
        *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD);
        for(uint8_t c=0;c<sx;c++)
        {
            index=c*sy + cc;
            *SSPDR = img[index];
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        }
    }
}

void ssd1306WriteChar(uint8_t row,uint8_t col,char chr)
{
    uint8_t fontIdx;
    setCursor(row,col*6);
    fontIdx = (uint8_t)chr - ' ';

    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // switch to data
    for (uint8_t c=0;c<5;c++)
    {
        *SSPDR = oled_font_5x7[fontIdx][c];
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    if(col==20) // clear last two columns
    {
        *SSPDR = 0x0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        *SSPDR = 0x0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }

}

void ssd1306WriteText(const char * str,uint8_t posH,uint8_t posV)
{
    uint8_t cnt = 0;
    uint8_t hCurrent=posH;
    while(*(str+cnt) != 0)
    {
        ssd1306WriteChar(posV,hCurrent,*(str+cnt));

        hCurrent += 1;

        cnt++;
    }
}