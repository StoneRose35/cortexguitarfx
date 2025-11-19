#include "avrProgrammer.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"
#include "drivers/display128x64.h"
#include "drivers/systick.h"


static volatile uint8_t programmerState;

void initAvrProgrammer()
{
    // get spi out of reset
    *RESETS |= (1 << RESETS_RESET_SPI0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_SPI0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_SPI0_LSB)) == 0);

    // wire up the spi
    *SSD1306_MOSI_PIN_CNTR = 1;
    *SSD1306_SCK_PIN_CNTR = 1;
    *SSD1306_CS_DISPLAY_PIN_CNTR = 1;
    *PROG_MISO_PIN_CNTR = 1;

	*GPIO_OE &= ~(1 << PROG_RESET);
	*GPIO_OUT &= ~(1 << PROG_RESET);

    *PROG_RESET_PIN_CNTR =  5; // function 5 (SIO)
    *(GPIO_OUT+1) = (1 << PROG_RESET);
	*GPIO_OE |= (1 << PROG_RESET);

    *SSD1306_CS_DISPLAY_PIN_CNTR = 5;
    *(GPIO_OUT + 1) = (1 << SSD1306_CS_DISPLAY);
    *GPIO_OE |= (1 << SSD1306_CS_DISPLAY);


    //set clock frequency of 200 MHz/(20*(99+1))
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (99 << SPI_SSPCR0_SCR_LSB);
    *SSPCPSR = 20;

    // configure control register 1: enable by setting synchronous operation
    *SSPCR1 = 1 << SPI_SSPCR1_SSE_LSB;
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

    *SSPDR = AVR_PROG_CMD_CHIP_ERASE_B1;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = AVR_PROG_CMD_CHIP_ERASE_B2;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = 0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *SSPDR = 0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

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
            *SSPDR = AVR_PROG_CMD_LOAD_PROGMEM_B1;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = 0;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = cnt & 0x3F;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = *(data + cnt) & 0xFF;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = AVR_PROG_CMD_LOAD_PROGMEM_B1 | 0x8;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = 0;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = cnt & 0x3F;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            *SSPDR = (*(data + cnt) >> 8) & 0xFF;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            cnt++;
        }
        *SSPDR = AVR_PROG_CMD_WRITE_PROGMEM_B1;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        *SSPDR = AVR_PROG_CMD_WRITE_PROGMEM_B2 | ((pageAddress) >> (6+2));
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        *SSPDR = AVR_PROG_CMD_WRITE_PROGMEM_B2 | pageAddress;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        waitUntilReady();
    }
    return 0;
}


uint16_t readAvrProgramMemoryHalfword(uint16_t address)
{
    uint16_t progmemHalfword;

    // flush receive fifo
    while((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))!=0)
    {
        (void)*SSPDR;
    }

    *SSPDR = AVR_PROG_CMD_READ_PROGMEM_B1; //command, low byte
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0x1F&(address >> 8); // address msb
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0xFF&(address); // address lsb
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR=0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    progmemHalfword = *SSPDR & 0xFF;
    *SSPDR = AVR_PROG_CMD_READ_PROGMEM_B1 | 0x8; //command, high byte
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0x1F&(address >> 8); // address msb
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0xFF&(address); // address lsb
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    (void)*SSPDR;  
    *SSPDR=0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    progmemHalfword |= (*SSPDR & 0xFF) << 8;
    return progmemHalfword;
}

uint8_t enableAvrProgrammingMode()
{
    while (IsDisplayUpdateOngoing()) waitSysticks(1);
    if (programmerState == AVR_PROG_STATE_PROGRAMMING)
    {
        return 0;
    }
    *(GPIO_OUT+2) = (1 << PROG_RESET); // reset low
    waitSysticks(2); // wait 20ms

    // flush receive fifo
    while((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))!=0)
    {
        (void)*SSPDR;
    }

    // send enable programming
    *SSPDR = AVR_PROG_CMD_ENABLE_PROG_B1; 
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    (void)*SSPDR; 
    *SSPDR = AVR_PROG_CMD_ENABLE_PROG_B2;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0x00;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    uint32_t echoedByte = *SSPDR;
    *SSPDR = 0x0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    if ((echoedByte & 0xFF) != AVR_PROG_CMD_ENABLE_PROG_B2)
    {
        return 0x1;
    }
    programmerState = AVR_PROG_STATE_PROGRAMMING;
    return 0x0;
}

uint8_t disableAvrProgrammingMode()
{
    *(GPIO_OUT+1) = (1 << PROG_RESET); // reset high
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
    while((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))!=0)
    {
        (void)*SSPDR;
    }

    *SSPDR = AVR_PROG_CMD_POLL_RDY_B1;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    (void)*SSPDR; 
    *SSPDR = 0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    notReady = (uint8_t)(*SSPDR & 0x01);
    while (notReady)
    { 
        *SSPDR = AVR_PROG_CMD_POLL_RDY_B1;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        notReady = (uint8_t)(*SSPDR & 0x01);
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
    while((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))!=0)
    {
        (void)*SSPDR;
    }

    for (uint8_t c=0;c<3;c++)
    {
        *SSPDR = AVR_PROG_CMD_READ_SIG_B1;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = c;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        (void)*SSPDR; 
        *SSPDR = 0;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        *(data+c) = *SSPDR & 0xFF;
    }
    return 0x0;
}
