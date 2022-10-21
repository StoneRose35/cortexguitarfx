
#include "ssd1306_display.h"
#include "systick.h"
#include "fonts/oled_font_5x7.h"
#include "stm32f446/stm32f446xx.h"
#include "stm32f446/stm32f446_cfg_pins.h"

static GPIO_TypeDef *gpio_cd;
static GPIO_TypeDef *gpio_reset;

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
    gpio->PUPDR &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr = gpio->AFR[(pinnr & 0xF)>>3];
    regbfr &= ~(0xF << ((pinnr & 0x7) << 2));
    regbfr |= alternateFunction << ((pinnr & 0x7) << 2);
    gpio->AFR[(pinnr & 0xF)>>3] = regbfr; 
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
    gpio_cd->OSPEEDR &= ~(3 << ((SSD1306_CD & 0xF)<<1));
    gpio_cd->PUPDR &= ~(3 << ((SSD1306_CD & 0xF)<<1));

    port = SSD1306_RESET >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_reset->MODER;
    regbfr &= ~(3 << ((SSD1306_RESET & 0xF)<<1));
    regbfr |= (1 << ((SSD1306_RESET & 0xF)<<1));
    gpio_reset->MODER=regbfr;
    gpio_reset->OTYPER &= ~(1 << (SSD1306_RESET & 0xF));
    gpio_reset->OSPEEDR &= ~(3 << ((SSD1306_RESET & 0xF)<<1));
    gpio_reset->PUPDR &= ~(3 << ((SSD1306_RESET & 0xF)<<1));

    config_spi_pin(SSD1306_MISO,6);
    config_spi_pin(SSD1306_MOSI,7);
    config_spi_pin(SSD1306_SCK,6);

    regbfr = SPI3->CR1;
    regbfr &= ~(7 << SPI_CR1_BR_Pos);
    regbfr |= (4 << SPI_CR1_BR_Pos) | (1 << SPI_CR1_MSTR_Pos) | (1 << SPI_CR1_SSM_Pos) | (1 << SPI_CR1_SSI_Pos); // 45MHz/32=1.40625 MHz, Master, software slave select
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
    
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF)+16)); // cd low
    SPI3->DR = 0x40; // set startline 0
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
    // set displayoffset 0
    SPI3->DR = 0xD3;
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
    SPI3->DR = 0x0;
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
    
    // send display on command
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF)+16)); // cd low
    SPI3->DR = 0xAF;
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
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
    gpio_cd->BSRR = (1 << ((SSD1306_CD & 0xF)+16)); // cd low
    // set column, low nibble
    SPI3->DR = ((col+2) & 0x0F);
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
    // set column, high nibble
    SPI3->DR  = 0x10 | ((col+2) >> 4);
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
    // set row / page
    SPI3->DR  = 0xB0 | row;
    while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
}

void ssd1306WriteChar(uint8_t row,uint8_t col,char chr)
{
    uint8_t fontIdx;
    setCursor(row,col*6);
    fontIdx = (uint8_t)chr - ' ';

    gpio_cd->BSRR = (1 << (SSD1306_CD & 0xF)); // cd high: data
    for (uint8_t c=0;c<5;c++)
    {
        SPI3->DR = oled_font_5x7[fontIdx][c];
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    }
    SPI3->DR = 0x0;
    while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    if(col==20) // clear last two columns
    {
        SPI3->DR = 0x0;
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
        SPI3->DR = 0x0;
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
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

void ssd1306DisplayByteArray(uint8_t row,uint8_t col,uint8_t *arr,uint16_t arrayLength)
{
    setCursor(row,col);
    gpio_cd->BSRR = (1 << (SSD1306_CD & 0xF)); // cd high: data
    for (uint16_t c=0;c<arrayLength;c++)
    {
        SPI3->DR = *(arr + c);
        while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0); 
    }
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
    uint16_t index;
    for(uint8_t cc=0;cc<sy;cc++)
    {
        setCursor(py+cc,px);
        gpio_cd->BSRR = (1 << (SSD1306_CD & 0xF)); // cd high: data
        for(uint8_t c=0;c<sx;c++)
        {
            index=c*sy + cc;
            SPI3->DR = img[index];
            while ((SPI3->SR & (1 << SPI_SR_TXE_Pos))==0);
        }
    }
}


void ssd1306ClearDisplay()
{
    for(uint8_t r=0;r<8;r++)
    {
        setCursor(r,0);
        for(uint8_t c=0;c<128;c++)
        {
            gpio_cd->BSRR = (1 << (SSD1306_CD & 0xF)); // cd high: data
            SPI3->DR  = 0x0;
            while ((SPI3->SR & (1 << SPI_SR_BSY_Pos))!=0); 
        }
    } 
}
