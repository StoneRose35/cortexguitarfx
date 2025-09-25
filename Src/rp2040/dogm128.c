#include "globalConfig.h"

#ifdef DOGM128_DISPLAY
#include "fonts/oled_font_5x7.h"
#include "drivers/dogm128.h"
#include "drivers/display128x64.h"
#include "drivers/systick.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"
#include "hardware/regs/dma.h"
#include "system.h"
#include "helpers.h"


static volatile uint8_t currentDmaRow=DOGM128_N_PAGES;
static volatile uint8_t * currentFrameBuffer=0;

extern uint32_t task;
#define SPI1_TXDR_BYTE  *((uint8_t*)&SPI1->TXDR) 


void dogm128SendCommand(uint8_t cmd)
{
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_CD);
    nop_wait(DOGM128_CS_DELAY);
    *SSPDR=cmd;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
}

void dogm128SendData(const uint8_t*data,uint8_t l)
{
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // cd high
    nop_wait(DOGM128_CS_DELAY);
    for(uint8_t c=0;c<l;c++)
    {
        *SSPDR=*(data+c);
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    }
}

void initDisplay()
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
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (SPI_SCR << SPI_SSPCR0_SCR_LSB);
    // configure clock divider
    *SSPCPSR = SPI_SSPCPSR;
    // configure control register 1: enable by setting synchronous operation
    *SSPCR1 = 1 << SPI_SSPCR1_SSE_LSB;
    //enable dma for the transmitter
    *SSPDMACR = (1 << SPI_SSPDMACR_TXDMAE_LSB);


    // reset high
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_RESET);
    //reset low
    *(GPIO_OUT + 2) = (1 << SSD1306_DISPLAY_RESET);
    waitSysticks(10);
    // reset high
    *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_RESET);
    waitSysticks(5);

    //dogm128Enable();
    waitSysticks(5);
    
    dogm128SendCommand(0x40);// set startline 0
    dogm128SendCommand(DOGM128_ADC_REVERSE);// ADC reverse, set A0 to flip display, set A1 for normal
    dogm128SendCommand(DOGM128_COM_DIRECTION); //Normal COM0-COM63, set C8 to reverse display, set C0 for standard
    dogm128SendCommand(0xA6); //display normal
    dogm128SendCommand(0xA2); // set bias 1/9 (Duty 1/65)
    dogm128SendCommand(0x2F); // Booster, regulator and follower on 
    dogm128SendCommand(0xF8); // internal booster to x4 
    dogm128SendCommand(0x00);  
    dogm128SendCommand(0x27); // constrast: voltage regulator set
    dogm128SendCommand(0x81); // constrast: electronic volume set 
    dogm128SendCommand(0x16); // was 0x16
    dogm128SendCommand(0xAC); // static indicator: no indicator
    dogm128SendCommand(0x00);
    dogm128SendCommand(0xAF); // finally: display on 
    waitSysticks(11);


    //dogm128Disable();

}

/**
 * @brief Set the Cursor object
 * 
 * @param row goes from 0 to SSD1306_DISPLAY_N_PAGES-1, defines the page to write
 * @param col columns, goes from 0 to SSD1306_DISPLAY_N_COLUMNS-1
 */
void setCursor(uint8_t row, uint8_t col)
{
    // set row / page
    dogm128SendCommand(0xB0 | row);
    // set column, high nibble
    dogm128SendCommand(0x10 | ((col+HORIZONTAL_OFFSET) >> 4));
    // set column, low nibble
    dogm128SendCommand((col+HORIZONTAL_OFFSET) & 0x0F);
}


void ClearDisplay()
{
    uint8_t zeroVals[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
    for(uint8_t r=0;r<DOGM128_N_PAGES;r++)
    {
        setCursor(r,0);
        dogm128SendData(zeroVals,128);
    }   
}

void CheckerBoardDisplay()
{
    uint8_t zeroVals[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                    };
    for(uint8_t r=0;r<DOGM128_N_PAGES;r++)
    {
        if (r & 1)
        {
            setCursor(r,0);
            dogm128SendData(zeroVals+16,128);
        }
        else
        {
            setCursor(r,0);
            dogm128SendData(zeroVals,128);
        }
    }   
}

/**
 * @brief fill a full or parts of a row with bytes
 * 
 * @param row the row from 0 to DOGM128_DISPLAY_N_PAGES-1
 * @param col starting column from 0 to DOGM128_DISPLAY_N_COLUMNS-1
 * @param arr the data array (lsb is on top)
 * @param arrayLength the length of the array
 */
void DisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength)
{
    setCursor(row,col);
    for (uint16_t c=0;c<arrayLength;c++)
    {
        dogm128SendData(arr+c,1);
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
void DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    uint16_t index;
    for(uint8_t cc=0;cc<sy;cc++)
    {
        setCursor(py+cc,px);
        for(uint8_t c=0;c<sx;c++)
        {
            index=c + cc*sx;
            dogm128SendData(img+index,1);

        }
    }
}

void DisplayWriteChar(char chr)
{
    uint8_t fontIdx;

    fontIdx = (uint8_t)chr - ' ';
    uint8_t zeros[2]={0,0};

    dogm128SendData(oled_font_5x7[fontIdx],5);
    dogm128SendData(zeros,1);
}


void DisplayWriteText(const char * str,uint8_t posH,uint8_t posV)
{
    uint8_t cnt = 0;
    uint8_t hCurrent=posH;
    uint8_t zeros[2]={0,0};
    setCursor(posV,posH*6);
    while(*(str+cnt) != 0)
    {
        DisplayWriteChar(*(str+cnt));
        hCurrent += 1;
        cnt++;
    }
    if (hCurrent==21) // last horizontal position written, clear the last two columns
    {
        dogm128SendData(zeros,2);
    }
}

/**
 * @brief writes a left-aligned text and fills the entire line
 * 
 * @param str 
 * @param posV 
 */
void DisplayWriteTextLine(const char * str,uint8_t posV)
{
    uint8_t cnt=0;
    setCursor(posV,0);
    uint8_t zeros[2]={0,0};
    while(*(str+cnt) != 0)
    {
        DisplayWriteChar(*(str+cnt));
        cnt++;
    }
    while(cnt < 21)
    {
        DisplayWriteChar(' ');
        cnt++;
    }
    dogm128SendData(zeros,2);
}

void DisplayWriteLineAsync(volatile uint8_t * data)
{
    *DMA_CH4_WRITE_ADDR = (uint32_t)SSPDR;
	*DMA_CH4_READ_ADDR = (uint32_t)data;
	*DMA_CH4_TRANS_COUNT = DOGM128_DISPLAY_N_COLUMNS;
	*DMA_CH4_CTRL_TRIG = (16 << DMA_CH4_CTRL_TRIG_TREQ_SEL_LSB) 
						| (1 << DMA_CH4_CTRL_TRIG_INCR_READ_LSB) 
						| (0 << DMA_CH4_CTRL_TRIG_DATA_SIZE_LSB) // byte wise transfer
						| (1 << DMA_CH4_CTRL_TRIG_EN_LSB);

}

void DisplayWriteNextLine(void)
{
    if (currentDmaRow < DOGM128_N_PAGES )
    {
        setCursor(currentDmaRow,0);
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
        *(GPIO_OUT + 1) = (1 << SSD1306_DISPLAY_CD); // cd high
        nop_wait(DOGM128_CS_DELAY);
        DisplayWriteLineAsync(currentFrameBuffer + currentDmaRow*DOGM128_DISPLAY_N_COLUMNS);
        currentDmaRow++;
    }

}

void DisplayWriteFramebufferAsync(uint8_t * fb)
{
    if(currentDmaRow==DOGM128_N_PAGES) // only write when previous transfer ended
    {
        currentDmaRow=0;
        currentFrameBuffer=fb;
        DisplayWriteNextLine();
    }
}

#endif