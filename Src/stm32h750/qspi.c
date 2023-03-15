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

#define QUADSPI_DR_BYTE (*((uint8_t*)(&QUADSPI->DR)))

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

void waitForStatus(uint32_t maskr,uint32_t matchr)
{
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
        // read status register until flag SR_WIP is cleared 
    QUADSPI->PSMKR = maskr;
    QUADSPI->PSMAR = matchr;
    QUADSPI->PIR = 0x10;
    QUADSPI->DLR = 1-1; // one byte of data
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) // instruction on one line
                | (2 << QUADSPI_CCR_FMODE_Pos) // automatic status polling  
                | (1 << QUADSPI_CCR_DMODE_Pos) 
                | READ_STATUS_REG_CMD;
    while((QUADSPI->SR & (1 << QUADSPI_SR_SMF_Pos)) ==0);
    QUADSPI->FCR = (1 << QUADSPI_FCR_CSMF_Pos);
}

void writeEnable()
{
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    // write enable
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) | WRITE_ENABLE_CMD;
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);
}

void writeEnableQpi()
{
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) | WRITE_ENABLE_CMD;
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);
}

void readManufacturerId(uint8_t * data)
{
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->DLR = 3-1;
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) 
                | (1 << QUADSPI_CCR_DMODE_Pos) 
                | (1 << QUADSPI_CCR_FMODE_Pos)
                | (READ_JEDEC_ID);
    for (uint8_t c=0;c<3;c++)
    {
        while((QUADSPI->SR & ((1 << QUADSPI_SR_TCF_Pos) | (1 << QUADSPI_SR_FTF_Pos)))==0);
        *(data+c)=QUADSPI_DR_BYTE;
    }
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);
}
void initQspi()
{
    uint8_t reg;
    uint8_t deviceId[5];

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

    QUADSPI->CR &=~(0x1F << QUADSPI_CR_FTHRES_Pos);
    QUADSPI->CR |= (3 << QUADSPI_CR_FTHRES_Pos);
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
// configure & switch on controller
    QUADSPI->CR |= (1 << QUADSPI_CR_PRESCALER_Pos)
                | (0 << QUADSPI_CR_SSHIFT_Pos)
                | (0 << QUADSPI_CR_FSEL_Pos)
                | (0 << QUADSPI_CR_DFM_Pos)
                | (1 << QUADSPI_CR_APMS_Pos); // stop autopolling on match


    
    QUADSPI->DCR |= (22  << QUADSPI_DCR_FSIZE_Pos) 
            | ((2-1) << QUADSPI_DCR_CSHT_Pos);

    QUADSPI->CR |= (1 << QUADSPI_CR_EN_Pos);

    // ---------------
    // Exit Quad Mode
    // ---------------
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) // instruction on four lines
                | (EXIT_QUAD_CMD );

    // -----------
    // Reset Memory
    // -----------
    // reset enable
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0); 
    QUADSPI->CCR = ((1 << QUADSPI_CCR_IMODE_Pos) | RESET_ENABLE_CMD);
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);

    // reset memory
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->CCR = ((1 << QUADSPI_CCR_IMODE_Pos) | RESET_MEMORY_CMD);
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);

    // read status register until flag SR_WIP is cleared 
    waitForStatus(IS25LP064A_SR_WIP,0);

    readManufacturerId(deviceId);

    // --------------------------
    // Configure the dummy cycles
    // --------------------------
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->DLR = 1-1;
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) | (1 << QUADSPI_CCR_DMODE_Pos) | SET_READ_PARAM_REG_CMD;
    QUADSPI_DR_BYTE = (uint8_t)0b11110000;
    
    QUADSPI->FCR = (1 << QUADSPI_FCR_CTCF_Pos);



    // -------------------------
    // Enable Quad Mode
    // -------------------------
    while((QUADSPI->SR & ((1 << QUADSPI_SR_TCF_Pos) | (1 << QUADSPI_SR_FTF_Pos)))==0);
    QUADSPI->DLR = 1-1;
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos)
                |(1 << QUADSPI_CCR_DMODE_Pos)
                |(1 << QUADSPI_CCR_FMODE_Pos)
                | READ_STATUS_REG_CMD;
    reg = QUADSPI_DR_BYTE;
    if ((reg & IS25LP064A_SR_QE)==0)
    {
        // write enable
        writeEnable();
        while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
        QUADSPI->DLR = 1-1;
        QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos)|(1 << QUADSPI_CCR_DMODE_Pos) | (WRITE_STATUS_REG_CMD);
        QUADSPI_DR_BYTE = reg | IS25LP064A_SR_QE;
        waitForStatus(IS25LP064A_SR_QE,IS25LP064A_SR_QE);
    }

    // send out QPIEN
    // from this point on only quad commands are allowed
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0);
    QUADSPI->DLR = 0; 
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) | (ENTER_QUAD_CMD);

}


// program a page of 256 bytes
void QspiProgramPage(uint32_t address,uint8_t*data)
{
    uint16_t c;

    writeEnableQpi();
    QUADSPI->DLR = 256-1;
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) // instruction on four lines
            | (PAGE_PROG_CMD << QUADSPI_CCR_INSTRUCTION_Pos)
            | (3 << QUADSPI_CCR_DMODE_Pos) // data mode: data on four lines
            | (0 << QUADSPI_CCR_DCYC_Pos) // 0 dummy cycles
            | (2 << QUADSPI_CCR_ADSIZE_Pos) // 24 bit address
            | (3 << QUADSPI_CCR_ADMODE_Pos) // address over 4 data lines
            | (0 << QUADSPI_CCR_FMODE_Pos);
    QUADSPI->AR = address;
    for(c=0;c<256;c++)
    {
        while((QUADSPI->SR & (1 << QUADSPI_SR_FTF_Pos))!=0);
        QUADSPI->DR = data[c];
    }
    waitForStatus(IS25LP064A_SR_WIP,0);
}

// erases a sector of 4 kbytes
void QspiEraseSector(uint32_t address)
{
    writeEnableQpi();
    QUADSPI->DLR = 0;
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) // instruction on four lines
            | (SECTOR_ERASE_QPI_CMD << QUADSPI_CCR_INSTRUCTION_Pos)
            | (3 << QUADSPI_CCR_DMODE_Pos) // data mode: data on four lines
            | (0 << QUADSPI_CCR_DCYC_Pos) // 0 dummy cycles
            | (2 << QUADSPI_CCR_ADSIZE_Pos) // 24 bit address
            | (3 << QUADSPI_CCR_ADMODE_Pos) // address over 4 data lines
            | (0 << QUADSPI_CCR_FMODE_Pos);
    QUADSPI->AR = address;
    waitForStatus(IS25LP064A_SR_WIP,0);
}

void QspiRead(uint32_t address,uint32_t nBytes,uint8_t * data)
{
    QUADSPI->DLR = nBytes;
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) // instruction on four lines
            | (SECTOR_ERASE_QPI_CMD << QUADSPI_CCR_INSTRUCTION_Pos)
            | (3 << QUADSPI_CCR_DMODE_Pos) // data mode: data on four lines
            | (8 << QUADSPI_CCR_DCYC_Pos) // 8 dummy cycles
            | (2 << QUADSPI_CCR_ADSIZE_Pos) // 24 bit address
            | (3 << QUADSPI_CCR_ADMODE_Pos) // address over 4 data lines
            | (1 << QUADSPI_CCR_FMODE_Pos); // indirect read
    QUADSPI->AR = address;
    for(uint32_t c=0;c<nBytes;c++)
    {
        while((QUADSPI->SR & (1 << QUADSPI_SR_FTF_Pos))!=0);
        *(data+c) = QUADSPI->DR;
    }
}

void setMemoryMappedMode()
{
    // disable qpi mode
    // since the first instruction will be in single line
    while((QUADSPI->SR & (1 << QUADSPI_SR_BUSY_Pos))!=0); 
    QUADSPI->DLR = 0;
    QUADSPI->CCR = (3 << QUADSPI_CCR_IMODE_Pos) // instruction on four lines
            | (EXIT_QUAD_CMD);
    while((QUADSPI->CR & (1 << QUADSPI_SR_BUSY_Pos))!=0);

    // define a read quad io command
    QUADSPI->ABR = 0xA0;
    QUADSPI->CCR = (1 << QUADSPI_CCR_IMODE_Pos) // instruction on one line
            | (QUAD_INOUT_FAST_READ_CMD << QUADSPI_CCR_INSTRUCTION_Pos)
            | (3 << QUADSPI_CCR_DMODE_Pos) // data mode: data on four lines
            | (6 << QUADSPI_CCR_DCYC_Pos) // 6 dummy cycles
            | (2 << QUADSPI_CCR_ADSIZE_Pos) // 24 bit address
            | (3 << QUADSPI_CCR_ADMODE_Pos) // address over 4 data lines
            | (3 << QUADSPI_CCR_FMODE_Pos) // memory-mapped mode
            | (1 << QUADSPI_CCR_SIOO_Pos) // instruction only during first command
            | (3 << QUADSPI_CCR_ABMODE_Pos);
}
