
#include "ssd1306_display.h"
#include "systick.h"
#include "fonts/oled_font_5x7.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "stm32h750/helpers.h"
#include <stdlib.h>

static GPIO_TypeDef *gpio_cd;
static GPIO_TypeDef *gpio_reset;
static GPIO_TypeDef *gpio_cs;

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
    gpio_cs->BSRR = (1 << ((SSD1306_CS & 0xF)+16)); // cs low
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF)+16)); // cd low
    short_nop_delay();
    SPI3->DR=cmd;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    short_nop_delay();
    gpio_cs->BSRR = (1 << ((SSD1306_CS & 0xF))); // cs high
}

void ssd1306SendData(const uint8_t*data,uint8_t l)
{
    gpio_cs->BSRR = (1 << ((SSD1306_CS & 0xF)+16)); // cs low
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF))); // cd high
    short_nop_delay();
    for(uint8_t c=0;c<l;c++)
    {
        SPI3->DR=*(data+c);
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    }
    short_nop_delay();
    gpio_cs->BSRR = (1 << ((SSD1306_CS & 0xF))); // cs high
}

void initSsd1306Display()
{
    uint32_t port;
    uint32_t regbfr;
    RCC->APB1ENR |= (1 << RCC_APB1ENR_SPI3EN_Pos);

    port = SSD1306_CD >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_cd=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_cd->MODER;
    regbfr &= ~(3 << ((SSD1306_CD & 0xF)<<1));
    regbfr |= (1 << ((SSD1306_CD & 0xF)<<1));
    gpio_cd->MODER=regbfr;
    gpio_cd->OTYPER &= ~(1 << (SSD1306_CD & 0xF));
    regbfr = gpio_cd->OSPEEDR;
    //regbfr &=~(3 << ((SSD1306_CD & 0xF)<<1));
    regbfr |= (3 << ((SSD1306_CD & 0xF)<<1));
    gpio_cd->OSPEEDR = regbfr;
    gpio_cd->PUPDR &= ~(3 << ((SSD1306_CD & 0xF)<<1));

    port = SSD1306_RESET >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_reset->MODER;
    regbfr &= ~(3 << ((SSD1306_RESET & 0xF)<<1));
    regbfr |= (1 << ((SSD1306_RESET & 0xF)<<1));
    gpio_reset->MODER=regbfr;
    gpio_reset->OTYPER &= ~(1 << (SSD1306_RESET & 0xF));
    regbfr = gpio_reset->OSPEEDR;
    //regbfr &=~(3 << ((SSD1306_RESET & 0xF)<<1));
    regbfr |= (3 << ((SSD1306_RESET & 0xF)<<1));
    gpio_reset->OSPEEDR = regbfr;
    gpio_reset->PUPDR &= ~(3 << ((SSD1306_RESET & 0xF)<<1));

    port = SSD1306_CS >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_cs=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_cs->MODER;
    regbfr &= ~(3 << ((SSD1306_CS & 0xF)<<1));
    regbfr |= (1 << ((SSD1306_CS & 0xF)<<1));
    gpio_cs->MODER=regbfr;
    gpio_cs->OTYPER &= ~(1 << (SSD1306_CS & 0xF));
    regbfr = gpio_cs->OSPEEDR;
    //regbfr &=~(3 << ((SSD1306_CS & 0xF)<<1));
    regbfr |= (3 << ((SSD1306_CS & 0xF)<<1));
    gpio_cs->OSPEEDR = regbfr;
    gpio_cs->PUPDR &= ~(3 << ((SSD1306_CS & 0xF)<<1));

    gpio_cs->BSRR = (1 << ((SSD1306_CS & 0xF))); // chip select high

    config_spi_pin(SSD1306_MISO,6);
    config_spi_pin(SSD1306_MOSI,7);
    config_spi_pin(SSD1306_SCK,6);

    regbfr = SPI3->CR1;
    regbfr &= ~(7 << SPI_CR1_BR_Pos);
    regbfr |= (3 << SPI_CR1_BR_Pos) | (1 << SPI_CR1_MSTR_Pos) | (1 << SPI_CR1_SSM_Pos) | (1 << SPI_CR1_SSI_Pos); // 45MHz/32=1.40625 MHz, Master, software slave select
    SPI3->CR1 = regbfr;
    SPI3->CR1 |= (1 << SPI_CR1_SPE_Pos);

    

    // reset high
    gpio_reset->BSRR = (1 << (SSD1306_RESET & 0xF));
    waitSysticks(1);
    //reset low
    gpio_reset->BSRR = (1 << ((SSD1306_RESET & 0xF)+16));
    waitSysticks(1);
    // reset high
    gpio_reset->BSRR = (1 << (SSD1306_RESET & 0xF));
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
}

void ssd1306_nop()
{
    SPI3->DR =0xE3;
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
}


/**
 * @brief Set the Cursor object
 * 
 * @param row goes from 0 to 7, defines the page to write
 * @param col columns, goes from 0 to 127
 */
void setCursor(uint8_t row, uint8_t col)
{              
    // set row / page
    ssd1306SendCommand(0xB0 | row);
    // set column, low nibble
    ssd1306SendCommand((col+2) & 0x0F);
    // set column, high nibble
    ssd1306SendCommand(0x10 | ((col+2) >> 4));
}

void ssd1306WriteChar(uint8_t row,uint8_t col,char chr)
{
    uint8_t fontIdx;
    uint8_t zeros[2]={0,0};
    setCursor(row,col*6);
    fontIdx = (uint8_t)chr - ' ';

    ssd1306SendData(oled_font_5x7[fontIdx],5);
    ssd1306SendData(zeros,1);
    if(col==20) // clear last two columns
    {
        ssd1306SendData(zeros,2);
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

/**
 * @brief writes a left-aligned text and fills the entire line
 * 
 * @param str 
 * @param posV 
 */
void ssd1306WriteTextLine(const char * str,uint8_t posV)
{
    uint8_t cnt=0;

    while(*(str+cnt) != 0)
    {
        ssd1306WriteChar(posV,cnt,*(str+cnt));
        cnt++;
    }
    while(cnt < 21)
    {
        ssd1306WriteChar(posV,cnt++,' ');
    }
}

void ssd1306DisplayByteArray(uint8_t row,uint8_t col,const uint8_t *arr,uint16_t arrayLength)
{
    setCursor(row,col);
    ssd1306SendData(arr,arrayLength);
}

void ssd1306DisplayImage(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF)+16)); // cd low: command

    // set vertical addressing mode
    SPI3->DR = 0x20;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    SPI3->DR = 0x02;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);

    // set column address
    SPI3->DR = 0x21;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);
    SPI3->DR =px;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);    
    SPI3->DR =px+sx;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);  

    // set page address
    SPI3->DR = 0x22;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);
    SPI3->DR =py;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);    
    SPI3->DR =py+sy;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);        

    uint16_t c=0;
    gpio_cd->BSRR = (1 << (SSD1306_CD & 0xF)); // cd high: data
    while(c<sx*sy)
    {
        SPI3->DR = *(img + c);
        c++;
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);
    }
}

void ssd1306DisplayImageStandardAdressing(uint8_t px,uint8_t py,uint8_t sx,uint8_t sy,uint8_t * img)
{
    uint8_t * imgBfr;
    uint16_t imgBfrCnt=0;
    imgBfr = (uint8_t*)malloc(sx*sy);
    // reorder image to diplay line by line
    for(uint8_t cc=0;cc<sy;cc++)
    {
        for (uint8_t c=0;c<sx;c++)
        {
            *(imgBfr + imgBfrCnt++) = *(img + c*sy + cc); 
        }
    }

    for(uint8_t cc=0;cc<sy;cc++)
    {
        setCursor(py+cc,px);
        ssd1306SendData(imgBfr + cc*sx,sx);
        //for (uint8_t c=0;c<sx;c++)
        //{
        //    ssd1306SendData(img + c*sy,1);
        //}
    }
    free(imgBfr);
}


void ssd1306ClearDisplay()
{
    const uint8_t zeros[128]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        };
    for(uint8_t r=0;r<8;r++)
    {
        setCursor(r,0);
        ssd1306SendData(zeros,128);
    } 
}
