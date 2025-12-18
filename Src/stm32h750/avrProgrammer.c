#include "avrProgrammer.h"
#include "drivers/systick.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "drivers/display128x64.h"
#include "drivers/systick.h"

#define SPI1_TXDR_BYTE  *((uint8_t*)&SPI1->TXDR) 
#define SPI1_RXDR_BYTE  *((uint8_t*)&SPI1->RXDR) 
static volatile uint8_t programmerState;


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

void initAvrProgrammer()
{
    // get spi out of reset
   uint32_t port;
    uint32_t regbfr;
    GPIO_TypeDef * gpio_reset;
    GPIO_TypeDef * gpio_cs;
    RCC->APB2ENR |= (1 << RCC_APB2ENR_SPI1EN_Pos);


    port = AVRPROG_RESET >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio_reset->MODER;
    regbfr &= ~(3 << ((AVRPROG_RESET & 0xF)<<1));
    regbfr |= (1 << ((AVRPROG_RESET & 0xF)<<1));
    gpio_reset->MODER=regbfr;
    gpio_reset->OTYPER &= ~(1 << (AVRPROG_RESET & 0xF));
    regbfr = gpio_reset->OSPEEDR;
    //regbfr &=~(3 << ((DISPLAY_RESET & 0xF)<<1));
    regbfr |= (3 << ((AVRPROG_RESET & 0xF)<<1));
    gpio_reset->OSPEEDR = regbfr;
    gpio_reset->PUPDR &= ~(3 << ((AVRPROG_RESET & 0xF)<<1));

    gpio_reset->BSRR = (1 << (AVRPROG_RESET & 0xF)); // reset /avr programmer cs high

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

    gpio_cs->BSRR = (1 << ((DISPLAY_CS & 0xF))); // cs high


    // configure spi pins
    config_spi_pin(AVRPROG_MISO,5);
    config_spi_pin(DISPLAY_MOSI,5);
    config_spi_pin(DISPLAY_SCK,5);




    SPI1->CR1 &= ~(1 << SPI_CR1_SPE_Pos);
    SPI1->IFCR = (1 << SPI_IFCR_SUSPC_Pos) | (1 << SPI_IFCR_TIFREC_Pos) | (1 << SPI_IFCR_OVRC_Pos) | (1 << SPI_IFCR_UDRC_Pos);

    regbfr = SPI1->CR1;
    regbfr |= (1 << SPI_CR1_SSI_Pos); // software slave select
    SPI1->CR1 = regbfr;

    regbfr = SPI1->CFG1;

    RCC->D2CFGR = (7 << RCC_D2CFGR_D2PPRE2_Pos) | (4 << RCC_D2CFGR_D2PPRE1_Pos); // D2PPRRE1 =2, D2PPRE2=16
    regbfr |= (6 << SPI_CFG1_MBR_Pos) | ((8-1) << SPI_CFG1_DSIZE_Pos); // 8 bits, 240MHz/16/128 as SPI clock,
    SPI1->CFG2 |= (1 << SPI_CFG2_MASTER_Pos) | (1 << SPI_CFG2_SSM_Pos);
    SPI1->CR1 |= (1 << SPI_CR1_SPE_Pos);
    SPI1->CR1 |= (1 << SPI_CR1_CSTART_Pos);

    programmerState = AVR_PROG_STATE_IDLE;
}


// check if a firmware identifier at a 256byte boundary matches an expected firmware number
// return 1 on match, 0 on no match
// and negative numbers on failure
int8_t matchAvrFirmwareVersion(uint32_t expectedFirmwareVersion)
{
    uint16_t size;
    uint8_t signatureBytes[3];
    uint16_t cnt=0;
    uint32_t firmwareVersion=0;
    if (enableAvrProgrammingMode() != 0)
    {
        return -1;
    }
    if (readSignatureBytes(signatureBytes)!= 0)
    {
        return -2;
    }
    if (signatureBytes[1] == 0x93)
    {
        size = 0x1000; // remember: 16-bit addressing
    }
    else if (signatureBytes[1] == 0x94)
    {
        size = 0x2000; // remember: 16-bit addressing
    }
    else
    {
        return -3;
    }

    while(cnt < size && firmwareVersion != expectedFirmwareVersion)
    {
        firmwareVersion = readAvrProgramMemoryHalfword(cnt);
        firmwareVersion |= readAvrProgramMemoryHalfword(cnt+1) << 16;
        cnt += 0x80; // remember: 16-bit addressing
    }
    if (firmwareVersion == expectedFirmwareVersion)
    {
        return 1;
    }
    return 0;
}


// download the AVR Flash image into an array
// the array must match the value read from the signature bytes
// 0x93 -> 8kB
// 0x94 -> 16kB
uint8_t downloadAvrFirmware(uint16_t * data)
{
    uint16_t size;
    uint8_t signatureBytes[3];
    if (enableAvrProgrammingMode() != 0)
    {
        return 1;
    }
    if (readSignatureBytes(signatureBytes)!= 0)
    {
        return 2;
    }
    if (signatureBytes[1] == 0x93)
    {
        size = 0x1000; // remember: 16-bit addressing
    }
    else if (signatureBytes[1] == 0x94)
    {
        size = 0x2000; // remember: 16-bit addressing
    }
    else
    {
        return 3;
    }
    for(uint16_t c=0;c<size;c++)
    {
        *(data+c) = readAvrProgramMemoryHalfword(c);
    }
    return 0;
}

// clear the AVR flash
uint8_t clearAvrFlash()
{
    if (enableAvrProgrammingMode() != 0)
    {
        return 1;
    }

    SPI1_TXDR_BYTE = AVR_PROG_CMD_CHIP_ERASE_B1;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    SPI1_TXDR_BYTE = AVR_PROG_CMD_CHIP_ERASE_B2;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    SPI1_TXDR_BYTE = 0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    SPI1_TXDR_BYTE = 0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 

    return waitUntilReady();
}

// upload a new avr Firmware onto the atmega88(p)
// chip erase must be performed beforehand
uint8_t uploadAvrFirmware(uint16_t *  data,uint16_t size)
{
    uint8_t pageSize;
    uint16_t maxSize;
    uint16_t pageAddress;
    uint8_t signatureBytes[3];
    uint16_t cnt=0;
    if (enableAvrProgrammingMode() != 0)
    {
        return 1; // programming mode not available, ic may be write locked
    }
    if (readSignatureBytes(signatureBytes)!= 0)
    {
        return 2; // unable to read signature bytes
    }
    if (signatureBytes[1] == 0x93)
    {
        maxSize = 0x1000; // remember: 16-bit addressing
        pageSize = 0x20;
    }
    else if (signatureBytes[1] == 0x94)
    {
        maxSize = 0x2000; // remember: 16-bit addressing
        pageSize = 0x40;
    }
    else
    {
        return 3; // unknown size signature
    }
    if (size > maxSize)
    {
        return 4; // image too large
    }

    while (cnt < size)
    {
        uint8_t pageLimit;
        if (size-cnt < pageSize)
        {
            pageLimit = size - cnt;
        }
        else
        {
            pageLimit = pageSize;
        }
        pageAddress = cnt;
        for (uint8_t q=0;q < pageLimit;q++)
        {
            SPI1_TXDR_BYTE = AVR_PROG_CMD_LOAD_PROGMEM_B1;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = 0;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = cnt & 0x3F;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = *(data + cnt) & 0xFF;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = AVR_PROG_CMD_LOAD_PROGMEM_B1 | 0x8;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = 0;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = cnt & 0x3F;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            SPI1_TXDR_BYTE = (*(data + cnt) >> 8) & 0xFF;
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            cnt++;
        }
        SPI1_TXDR_BYTE = AVR_PROG_CMD_WRITE_PROGMEM_B1;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        SPI1_TXDR_BYTE = AVR_PROG_CMD_WRITE_PROGMEM_B2 | ((pageAddress) >> (6+2));
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        SPI1_TXDR_BYTE = AVR_PROG_CMD_WRITE_PROGMEM_B2 | pageAddress;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        waitUntilReady();
    }
    return 0;
}


uint16_t readAvrProgramMemoryHalfword(uint16_t address)
{
    uint16_t progmemHalfword;

    // flush receive fifo
    while((SPI1->SR & (3 << SPI_SR_RXPLVL_Pos))!=0)
    {
        (void)SPI1_RXDR_BYTE;
    }

    SPI1_TXDR_BYTE = AVR_PROG_CMD_READ_PROGMEM_B1; //command, low byte
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0x1F&(address >> 8); // address msb
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE =  0xFF&(address); // address lsb
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE =0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    progmemHalfword = SPI1_RXDR_BYTE & 0xFF;
    SPI1_TXDR_BYTE = AVR_PROG_CMD_READ_PROGMEM_B1 | 0x8; //command, high byte
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0x1F&(address >> 8); // address msb
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0xFF&(address); // address lsb
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE =0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    progmemHalfword |= (SPI1_RXDR_BYTE & 0xFF) << 8;
    return progmemHalfword;
}

uint8_t enableAvrProgrammingMode()
{
    GPIO_TypeDef * gpio_reset;
    uint32_t port = AVRPROG_RESET >> 4;
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    while (IsDisplayUpdateOngoing()) waitSysticks(1);
    if (programmerState == AVR_PROG_STATE_PROGRAMMING)
    {
        return 0;
    }
    gpio_reset->BSRR = (1 << ((AVRPROG_RESET & 0xF)+16)); // reset low
    waitSysticks(2); // wait 20ms

    // flush receive fifo
    while((SPI1->SR & (3 << SPI_SR_RXPLVL_Pos))!=0)
    {
        (void)SPI1_RXDR_BYTE;
    }

    // send enable programming
    SPI1_TXDR_BYTE = AVR_PROG_CMD_ENABLE_PROG_B1; 
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = AVR_PROG_CMD_ENABLE_PROG_B2;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0x00;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    uint32_t echoedByte = SPI1_TXDR_BYTE;
    SPI1_TXDR_BYTE = 0x0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    if ((echoedByte & 0xFF) != AVR_PROG_CMD_ENABLE_PROG_B2)
    {
        return 0x1;
    }
    programmerState = AVR_PROG_STATE_PROGRAMMING;
    return 0x0;
}

uint8_t disableAvrProgrammingMode()
{
    GPIO_TypeDef * gpio_reset;
    uint32_t port = AVRPROG_RESET >> 4;
    gpio_reset=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio_reset->BSRR = (1 << (AVRPROG_RESET & 0xF)); 
    programmerState = AVR_PROG_STATE_IDLE;
    return 0;
}

uint8_t waitUntilReady()
{
    uint8_t notReadyCnt=0;
    uint8_t notReady = 1;
    if (enableAvrProgrammingMode() != 0)
    {
        return 0x1;
    }

    // flush receive fifo
    while((SPI1->SR & (3 << SPI_SR_RXPLVL_Pos))!=0)
    {
        (void)SPI1_RXDR_BYTE;
    }

    SPI1_TXDR_BYTE = AVR_PROG_CMD_POLL_RDY_B1;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    (void)SPI1_RXDR_BYTE;
    SPI1_TXDR_BYTE = 0;
    while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
    notReady = (uint8_t)(SPI1_RXDR_BYTE & 0x01);
    while (notReady)
    { 
        SPI1_TXDR_BYTE = AVR_PROG_CMD_POLL_RDY_B1;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        notReady = (uint8_t)(SPI1_RXDR_BYTE & 0x01);
        notReadyCnt++;
    }
    return notReadyCnt;
}

uint8_t readSignatureBytes(uint8_t * data)
{
    if (enableAvrProgrammingMode() != 0)
    {
        return 0x1;
    }
    // flush receive fifo
    while((SPI1->SR & (3 << SPI_SR_RXPLVL_Pos))!=0)
    {
        (void)SPI1_RXDR_BYTE;
    }


    for (uint8_t c=0;c<3;c++)
    {
        SPI1_TXDR_BYTE = AVR_PROG_CMD_READ_SIG_B1;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = c;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        (void)SPI1_RXDR_BYTE;
        SPI1_TXDR_BYTE = 0;
        while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
        *(data+c) = SPI1_RXDR_BYTE & 0xFF;
    }
    return 0x0;
}
