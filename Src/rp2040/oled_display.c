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
#include "drivers/oled_display.h"
#include "drivers/systick.h"
#include "fonts/oled_font_5x7.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "hardware/regs/dma.h"

static volatile uint8_t currentDmaRow=SSD1306_DISPLAY_N_PAGES;
static volatile uint8_t * currentFrameBuffer=0;


void initOledDisplay()
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
    //enable dma for the transmitter
    *SSPDMACR = (1 << SPI_SSPDMACR_TXDMAE_LSB);




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
    #ifdef SH1107
    *SSPDR = 0xDC; // set startline 0
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    #endif
    #ifdef SH1106
    *SSPDR = 0x40; // set startline 0
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    #endif


    // set displayoffset 0
    *SSPDR = 0xD3;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // COM remap to vertically flip the display
    #ifdef VERTICAL_FLIP
    *SSPDR = 0xC8;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    #endif
    #ifdef HORIZONTAL_FLIP
    // columns remap (flip horizontal)
    *SSPDR = 0xA1;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    #endif

    // send display on command
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    *SSPDR = 0xAF;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    waitSysticks(11);

    // enable interrupt on channel4
    *DMA_INTE0 |= (1 << 4);
}

/**
 * @brief Set the Cursor object
 * 
 * @param row goes from 0 to SSD1306_DISPLAY_N_PAGES-1, defines the page to write
 * @param col columns, goes from 0 to SSD1306_DISPLAY_N_COLUMNS-1
 */
void setCursor(uint8_t row, uint8_t col)
{
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    // set row / page
    *SSPDR = 0xB0 | row;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    // set column, low nibble
    *SSPDR = ((col+HORIZONTAL_OFFSET) & 0x0F);
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    // set column, high nibble
    *SSPDR = 0x10 | ((col+HORIZONTAL_OFFSET) >> 4);
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
}

void OledClearDisplay()
{
    for(uint8_t r=0;r<SSD1306_DISPLAY_N_PAGES;r++)
    {
        for(uint8_t c=0;c<SSD1306_DISPLAY_N_COLUMNS;c++)
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
 * @param row the row from 0 to SSD1306_DISPLAY_N_PAGES-1
 * @param col starting column from 0 to SSD1306_DISPLAY_N_COLUMNS-1
 * @param arr the data array (lsb is on top)
 * @param arrayLength the length of the array
 */
void OledDisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength)
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
 * @param px x value of the top left position (0 to SSD1306_DISPLAY_N_COLUMNS-1)
 * @param py y values of the top left position (0 to SSD1306_DISPLAY_N_PAGES-1)
 * @param sx x size of the image
 * @param sy y size of the image in pages (8 bit)
 * @param img the image data, the number of bytes must be sx*sy
 */
void OledDisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
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
void OledDisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    uint16_t index;
    for(uint8_t cc=0;cc<sy;cc++)
    {
        setCursor(py+cc,px);
        *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD);
        for(uint8_t c=0;c<sx;c++)
        {
            index=c + cc*sx;
            *SSPDR = img[index];
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        }
    }
}

void OledWriteChar(char chr)
{
    uint8_t fontIdx;

    fontIdx = (uint8_t)chr - ' ';

    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // switch to data
    for (uint8_t c=0;c<5;c++)
    {
        *SSPDR = oled_font_5x7[fontIdx][c];
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
}

void OledWriteText(const char * str,uint8_t posH,uint8_t posV)
{
    uint8_t cnt = 0;
    uint8_t hCurrent=posH;
    setCursor(posV,posH*6);
    while(*(str+cnt) != 0)
    {
        OledWriteChar(*(str+cnt));
        hCurrent += 1;
        cnt++;
    }
    if (hCurrent==21) // last horizontal position written, clear the last two columns
    {
        *SSPDR = 0x0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        *SSPDR = 0x0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    }
}

/**
 * @brief writes a left-aligned text and fills the entire line
 * 
 * @param str 
 * @param posV 
 */
void OledWriteTextLine(const char * str,uint8_t posV)
{
    uint8_t cnt=0;
    setCursor(posV,0);
    while(*(str+cnt) != 0)
    {
        OledWriteChar(*(str+cnt));
        cnt++;
    }
    while(cnt < 21)
    {
        OledWriteChar(' ');
        cnt++;
    }
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
}

void OledWriteLineAsync(volatile uint8_t * data)
{
    *DMA_CH4_WRITE_ADDR = (uint32_t)SSPDR;
	*DMA_CH4_READ_ADDR = (uint32_t)data;
	*DMA_CH4_TRANS_COUNT = SSD1306_DISPLAY_N_COLUMNS;
	*DMA_CH4_CTRL_TRIG = (16 << DMA_CH4_CTRL_TRIG_TREQ_SEL_LSB) 
						| (1 << DMA_CH4_CTRL_TRIG_INCR_READ_LSB) 
						| (0 << DMA_CH4_CTRL_TRIG_DATA_SIZE_LSB) // byte wise transfer
						| (1 << DMA_CH4_CTRL_TRIG_EN_LSB);

    *DMA_INTE0 |= (1 << 4);
}

void OledWriteNextLine(void)
{
    if (currentDmaRow <SSD1306_DISPLAY_N_PAGES )
    {
        setCursor(currentDmaRow,0);
        *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD);
        OledWriteLineAsync(currentFrameBuffer + currentDmaRow*SSD1306_DISPLAY_N_COLUMNS);
        currentDmaRow++;
    }
}

void OledwriteFramebufferAsync(uint8_t * fb)
{
    while(currentDmaRow<SSD1306_DISPLAY_N_PAGES); // block until previous transfer is done
    currentDmaRow=0;
    currentFrameBuffer=fb;
    OledWriteNextLine();
}
