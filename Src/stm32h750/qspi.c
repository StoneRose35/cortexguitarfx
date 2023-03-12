#include "qspi.h"
#include "stm32h750/stm32h750xx.h"

/*
&config_.pin_config.io0,
&config_.pin_config.io1,
&config_.pin_config.io2,
&config_.pin_config.io3,
&config_.pin_config.clk,
&config_.pin_config.ncs};

GPIO_AF10_QUADSPI,
GPIO_AF10_QUADSPI,
GPIO_AF9_QUADSPI,
GPIO_AF9_QUADSPI,
GPIO_AF9_QUADSPI,
GPIO_AF10_QUADSPI};
*/
typedef struct 
{
    const uint8_t port;
    const uint8_t pin;
    const uint8_t af;
} QspiPinType;


const QspiPinType qspiPins[6] = {
    { //io 0
        .af =10,
        .pin = 8,
        .port = 'F' - 'A'
    },
    { //io 1
        .af =10,
        .pin = 9,
        .port = 'F' - 'A'
    },
    { //io 2
        .af =9,
        .pin = 7,
        .port = 'F' - 'A'
    },
    { //io 3
        .af =9,
        .pin = 6,
        .port = 'F' - 'A'
    },
    { //clk
        .af =9,
        .pin = 10,
        .port = 'F' - 'A'
    },
    { // ncs
        .af = 10,
        .pin = 6,
        .port = 'G' - 'A'
    }
};

void setQspiGpio(const QspiPinType * pinType)
{
    GPIO_TypeDef *gpio;
    uint32_t moderBfr,otypeBfr,pupdBfr,ospeedBfr,afrlBfr,afrhBfr;
    RCC->AHB4ENR |= (1 << pinType->port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + pinType->port*0x400);

    moderBfr = gpio->MODER;
    moderBfr &= ~(3 << ((pinType->pin & 0xF)<<1));
    moderBfr |= (2 << ((pinType->pin & 0xF)<<1));
    gpio->MODER = moderBfr;

    otypeBfr = gpio->OTYPER;
    otypeBfr &= ~(1 << (pinType->pin & 0xF));
    gpio->OTYPER = otypeBfr;

    pupdBfr = gpio->PUPDR;
    pupdBfr &= ~(3 << ((pinType->pin & 0xF)<<1));
    gpio->PUPDR = pupdBfr;

    ospeedBfr = gpio->OSPEEDR;
    ospeedBfr |= (3 << ((pinType->pin & 0xF)<<1));
    gpio->OSPEEDR = ospeedBfr;

    afrlBfr = gpio->AFR[0];
    afrhBfr = gpio->AFR[1];
    if (((pinType->pin & 0xF)>>3) != 0)
    {
        afrhBfr &= ~(0xF << ((pinType->pin & 0x7) << 2));
        afrhBfr |= pinType->af << ((pinType->pin & 0x7) << 2);
    }
    else
    {
        afrlBfr &= ~(0xF << ((pinType->pin & 0x7) << 2));
        afrlBfr |= pinType->af << ((pinType->pin & 0x7) << 2);
    }   
    gpio->AFR[0] = afrlBfr;
    gpio->AFR[1] = afrhBfr;

}

void initQspi()
{
    RCC->AHB3ENR |= (1 << RCC_AHB3ENR_QSPIEN_Pos);

    // wire up pins
    //RCC->AHB4ENR |= (1 << RCC_AHB4ENR_GPIOGEN_Pos) | 
    //                (1 << RCC_AHB4ENR_GPIOFEN_Pos) |
    //                (1 << RCC_AHB4ENR_GPIOEEN_Pos) |
    //                (1 << RCC_AHB4ENR_GPIOBEN_Pos);
    for(uint8_t c=0;c<6;c++)
    {
        setQspiGpio(&qspiPins[c]);
    }

    // configure & switch on controller
    QUADSPI->DCR |= (23  << QUADSPI_DCR_FSIZE_Pos) 
            | ((2-1) << QUADSPI_DCR_CSHT_Pos);

    QUADSPI->CR |= ((1-1) << QUADSPI_CR_PRESCALER_Pos)
                | (1 << QUADSPI_CR_FTHRES_Pos)
                | (0 << QUADSPI_CR_SSHIFT_Pos)
                | (0 << QUADSPI_CR_FSEL_Pos)
                | (0 << QUADSPI_CR_DFM_Pos)
                | (1 << QUADSPI_CR_EN_Pos);

    // -----------
    // Reset Memory
    // -----------
    // reset enable
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) // instruction on one line
                | (RESET_ENABLE_CMD << QUADSPI_CCR_INSTRUCTION_Pos);
    while((QUADSPI->CR & (1 << QUADSPI_SR_BUSY_Pos))!=0);

    // reset memory
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) // instruction on one line
                | (RESET_MEMORY_CMD << QUADSPI_CCR_INSTRUCTION_Pos);
    while((QUADSPI->CR & (1 << QUADSPI_SR_BUSY_Pos))!=0);

    // read status register until flag SR_WIP is cleared (probably)
    QUADSPI->PSMKR = IS25LP064A_SR_WIP; // mask the "write in Progress" bit
    QUADSPI->PSMAR = 0; // match if cleared
    QUADSPI->PIR = 0x10;
    QUADSPI->DLR = 1; // one byte of data
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) // instruction on one line
                | (READ_STATUS_REG_CMD << QUADSPI_CCR_INSTRUCTION_Pos)
                | (2 << QUADSPI_CCR_FMODE_Pos) // automatic status polling  
                | (1 << QUADSPI_CCR_DMODE_Pos); // data mode: data one line
    while((QUADSPI->SR & (1 << QUADSPI_SR_SMF_Pos)) ==0);

    // --------------------------
    // Configure the dummy cycles
    // --------------------------

    // -------------------------
    // Enable Quad Mode
    // -------------------------

}