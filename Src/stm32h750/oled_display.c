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

 typedef void oled_display;
#ifdef JOYIT_128X64_DISPLAY
#include "globalConfig.h"


#include "drivers/oled_display.h"
#include "drivers/display128x64.h"
#include "fonts/oled_font_5x7.h"
#include "drivers/systick.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "stm32h750/helpers.h"
#include "system.h"


static volatile uint8_t currentDmaRow=SSD1306_DISPLAY_N_PAGES;
static volatile uint8_t * currentFrameBuffer=0;
static GPIO_TypeDef *gpio_cd;
static GPIO_TypeDef *gpio_reset;
static GPIO_TypeDef *gpio_cs;
extern uint32_t task;
#define SPI1_TXDR_BYTE  *((uint8_t*)&SPI1->TXDR) 


void DMA1_Stream3_IRQHandler(void) 
{
    // clear dma transfer complete flag
    DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF3_Pos);
    task |= (1 << TASK_DISPLAY_NEXT_LINE);
}

static void config_spi_pin(uint8_t pinnr,uint8_t alternateFunction)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (2 << ((pinnr & 0xF)<<1));
    gpio->MODER=regbfr;
    regbfr = gpio->OSPEEDR;
    //regbfr &=~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (3 << ((pinnr & 0xF)<<1));
    gpio->OSPEEDR = regbfr;
    gpio->PUPDR &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr = gpio->AFR[(pinnr & 0xF)>>3];
    regbfr &= ~(0xF << ((pinnr & 0x7) << 2));
    regbfr |= alternateFunction << ((pinnr & 0x7) << 2);
    gpio->AFR[(pinnr & 0xF)>>3] = regbfr; 
}

void ssd1306SendCommand(uint8_t cmd)
{
    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF)+16)); // cs low
    gpio_cd->BSRR = (1 << ((DISPLAY_CD & 0xF)+16)); // cd low
    short_nop_delay();
    SPI1_TXDR_BYTE=cmd;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    short_nop_delay();
    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF))); // cs high
}

void ssd1306SendData(const uint8_t*data,uint8_t l)
{
    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF)+16)); // cs low
    gpio_cd->BSRR = (1 << ((DISPLAY_CD & 0xF))); // cd high
    short_nop_delay();
    for(uint8_t c=0;c<l;c++)
    {
        SPI1_TXDR_BYTE=*(data+c);
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    }
    short_nop_delay();
    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF))); // cs high
}

void initDisplay()
{
   uint32_t port;
    uint32_t regbfr;
    RCC->APB2ENR |= (1 << RCC_APB2ENR_SPI1EN_Pos);

    port = DISPLAY_CD >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio_cd=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_cd->MODER;
    regbfr &= ~(3 << ((DISPLAY_CD & 0xF)<<1));
    regbfr |= (1 << ((DISPLAY_CD & 0xF)<<1));
    gpio_cd->MODER=regbfr;
    gpio_cd->OTYPER &= ~(1 << (DISPLAY_CD & 0xF));
    regbfr = gpio_cd->OSPEEDR;
    //regbfr &=~(3 << ((DISPLAY_CD & 0xF)<<1));
    regbfr |= (3 << ((DISPLAY_CD & 0xF)<<1));
    gpio_cd->OSPEEDR = regbfr;
    gpio_cd->PUPDR &= ~(3 << ((DISPLAY_CD & 0xF)<<1));

    port = DISPLAY_RESET >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_reset->MODER;
    regbfr &= ~(3 << ((DISPLAY_RESET & 0xF)<<1));
    regbfr |= (1 << ((DISPLAY_RESET & 0xF)<<1));
    gpio_reset->MODER=regbfr;
    gpio_reset->OTYPER &= ~(1 << (DISPLAY_RESET & 0xF));
    regbfr = gpio_reset->OSPEEDR;
    //regbfr &=~(3 << ((DISPLAY_RESET & 0xF)<<1));
    regbfr |= (3 << ((DISPLAY_RESET & 0xF)<<1));
    gpio_reset->OSPEEDR = regbfr;
    gpio_reset->PUPDR &= ~(3 << ((DISPLAY_RESET & 0xF)<<1));

    port = DISPLAY_CS >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_cs=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_cs->MODER;
    regbfr &= ~(3 << ((DISPLAY_CS & 0xF)<<1));
    regbfr |= (1 << ((DISPLAY_CS & 0xF)<<1));
    gpio_cs->MODER=regbfr;
    gpio_cs->OTYPER &= ~(1 << (DISPLAY_CS & 0xF));
    regbfr = gpio_cs->OSPEEDR;
    //regbfr &=~(3 << ((DISPLAY_CS & 0xF)<<1));
    regbfr |= (3 << ((DISPLAY_CS & 0xF)<<1));
    gpio_cs->OSPEEDR = regbfr;
    gpio_cs->PUPDR &= ~(3 << ((DISPLAY_CS & 0xF)<<1));

    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF))); // chip select high

    config_spi_pin(DISPLAY_MISO,5);
    config_spi_pin(DISPLAY_MOSI,5);
    config_spi_pin(DISPLAY_SCK,5);


    DMA1_Stream3->PAR=(uint32_t)&(SPI1->TXDR);
    DMA1_Stream3->M0AR=(uint32_t)0;
    DMA1_Stream3->M1AR=(uint32_t)0;
    DMA1_Stream3->CR = (0 << DMA_SxCR_MSIZE_Pos) | (0 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                       (0 << DMA_SxCR_CIRC_Pos) | (1 << DMA_SxCR_TCIE_Pos) | (1 << DMA_SxCR_DIR_Pos) |
                       (0 << DMA_SxCR_HTIE_Pos);

    DMA1_Stream3->NDTR=SSD1306_DISPLAY_N_COLUMNS;
    DMAMUX1_Channel3->CCR = ((38) << DMAMUX_CxCR_DMAREQ_ID_Pos); //spi1_tx_dma 


    SPI1->CR1 &= ~(1 << SPI_CR1_SPE_Pos);
    SPI1->IFCR = (1 << SPI_IFCR_SUSPC_Pos) | (1 << SPI_IFCR_TIFREC_Pos) | (1 << SPI_IFCR_OVRC_Pos) | (1 << SPI_IFCR_UDRC_Pos);

    regbfr = SPI1->CR1;
    regbfr |= (1 << SPI_CR1_SSI_Pos); // software slave select
    SPI1->CR1 = regbfr;

    regbfr = SPI1->CFG1;
    RCC->D2CFGR = (4 << RCC_D2CFGR_D2PPRE2_Pos) | (4 << RCC_D2CFGR_D2PPRE1_Pos); // D2PPRRE1 =2, D2PPRE2=2
    regbfr |= (1 << SPI_CFG1_MBR_Pos) | ((8-1) << SPI_CFG1_DSIZE_Pos) | (1 << SPI_CFG1_TXDMAEN_Pos); // 8 bits, 25MHz/4 as SPI clock, DMA enable for TX
    SPI1->CFG1 = regbfr;
    SPI1->CFG2 |= (1 << SPI_CFG2_MASTER_Pos) | (1 << SPI_CFG2_SSM_Pos);
    SPI1->CR1 |= (1 << SPI_CR1_SPE_Pos);
    SPI1->CR1 |= (1 << SPI_CR1_CSTART_Pos);


    // reset high
    gpio_reset->BSRR = (1 << (DISPLAY_RESET & 0xF));
    waitSysticks(1);
    //reset low
    gpio_reset->BSRR = (1 << ((DISPLAY_RESET & 0xF)+16));
    waitSysticks(1);
    // reset high
    gpio_reset->BSRR = (1 << (DISPLAY_RESET & 0xF));
    waitSysticks(1);


    // manually set display offset and  startline since these two values turned out to be wrong after reset
    ssd1306SendCommand(0x40);// set startline 0
    ssd1306SendCommand(0xD3);// set displayoffset 0
    ssd1306SendCommand(0x0);

    // column remap
    ssd1306SendCommand(0xA1);
    // flip common output scan direction
    ssd1306SendCommand(0xC8);
    
    // send display on command
    ssd1306SendCommand(0xAF);
    waitSysticks(11);


    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
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
    ssd1306SendCommand(0xB0 | row);
    // set column, low nibble
    ssd1306SendCommand((col+HORIZONTAL_OFFSET) & 0x0F);
    // set column, high nibble
    ssd1306SendCommand(0x10 | ((col+HORIZONTAL_OFFSET) >> 4));
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
    for(uint8_t r=0;r<SSD1306_DISPLAY_N_PAGES;r++)
    {
        setCursor(r,0);
        ssd1306SendData(zeroVals,128);
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
void DisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength)
{
    setCursor(row,col);
    for (uint16_t c=0;c<arrayLength;c++)
    {
        ssd1306SendData(arr+c,1);
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
    gpio_cd->BSRR = (1 << ((DISPLAY_CD & 0xF)+16)); // cd low: command

    // set vertical addressing mode
    SPI1_TXDR_BYTE = 0x20;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    SPI1_TXDR_BYTE = 0x02;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);

    // set column address
    SPI1_TXDR_BYTE = 0x21;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);
    SPI1_TXDR_BYTE =px;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);    
    SPI1_TXDR_BYTE =px+sx;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);  

    // set page address
    SPI1_TXDR_BYTE = 0x22;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);
    SPI1_TXDR_BYTE =py;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);    
    SPI1_TXDR_BYTE =py+sy;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0);        

    uint16_t c=0;
    gpio_cd->BSRR = (1 << (DISPLAY_CD & 0xF)); // cd high: data
    while(c<sx*sy)
    {
        SPI1_TXDR_BYTE = *(img + c);
        c++;
        while ((SPI3->SR & (1 << SPI_SR_TXC_Pos))==0);
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
            ssd1306SendData(img+index,1);

        }
    }
}

void DisplayWriteChar(char chr)
{
    uint8_t fontIdx;

    fontIdx = (uint8_t)chr - ' ';
    uint8_t zeros[2]={0,0};

    ssd1306SendData(oled_font_5x7[fontIdx],5);
    ssd1306SendData(zeros,1);
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
        ssd1306SendData(zeros,2);
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
    ssd1306SendData(zeros,2);
}

void DisplayWriteLineAsync(volatile uint8_t * data)
{
    DMA1_Stream3->CR &= ~(1 << DMA_SxCR_EN_Pos);
    DMA1_Stream3->M0AR=(uint32_t)data;
    DMA1_Stream3->M1AR=(uint32_t)data;
    DMA1_Stream3->CR |= (1 << DMA_SxCR_EN_Pos);

}

void DisplayWriteNextLine(void)
{
    if (currentDmaRow == SSD1306_DISPLAY_N_PAGES)
    {
        gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF))); // cs high
        short_nop_delay();
    }
    else if (currentDmaRow <SSD1306_DISPLAY_N_PAGES )
    {
        setCursor(currentDmaRow,0);
        gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF)+16)); // cs low
        gpio_cd->BSRR = (1 << ((DISPLAY_CD & 0xF))); // cd high
        short_nop_delay();
        DisplayWriteLineAsync(currentFrameBuffer + currentDmaRow*SSD1306_DISPLAY_N_COLUMNS);
        currentDmaRow++;
    }

}

void DisplayWriteFramebufferAsync(uint8_t * fb)
{
    if(currentDmaRow==SSD1306_DISPLAY_N_PAGES) // only write when previous transfer ended
    {
        currentDmaRow=0;
        currentFrameBuffer=fb;
        DisplayWriteNextLine();
    }
}

uint8_t IsDisplayUpdateOngoing()
{
    return !(DMA1_Stream3->NDTR == 0 &&
         (SPI1->SR & (1 << SPI_SR_TXC_Pos))==0 &&
          currentDmaRow == DOGM128_DISPLAY_N_COLUMNS);
}

#endif