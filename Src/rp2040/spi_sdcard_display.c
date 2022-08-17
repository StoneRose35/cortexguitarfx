
#include "spi_sdcard_display.h"
#include "systick.h"
#include "dma.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/spi.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/pwm.h"
#include "hardware/rp2040_registers.h"

/**
 * @brief Driver for https://www.adafruit.com/product/358
 * 1.8" Color TFT LCD display with MicroSD Card Breakout - ST7735R
 */

void initSpi()
{
    // get spi out of reset
    *RESETS |= (1 << RESETS_RESET_SPI0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_SPI0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_SPI0_LSB)) == 0);

    // get pads out of reset
    *RESETS |= (1 << RESETS_RESET_PADS_BANK0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_PADS_BANK0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PADS_BANK0_LSB)) == 0);

    // wire up the spi
    *MISO_PIN_CNTR = 1;
    *MOSI_PIN_CNTR = 1;
    *SCK_PIN_CNTR = 1;

    // switch off sd card cs when initializing the spi interface
    *GPIO_OE |= (1 << CS_SDCARD);
	*(GPIO_OUT + 1) = (1 << CS_SDCARD);
    *CS_SDCARD_PIN_CNTR = 5;

    // control the display cs pin by software 
    *GPIO_OE |= (1 << CS_DISPLAY);
	*(GPIO_OUT + 1) = (1 << CS_DISPLAY);
    *CS_DISPLAY_PIN_CNTR = 5;

    // enable pullups on miso
    *MISO_PAD_CNTR &= ~(1 << PADS_BANK0_GPIO0_PDE_LSB);
    *MISO_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_PUE_LSB);    

    // enable output on reset and command/data pin
    *GPIO_OE |= (1 << DISPLAY_RESET);
    *GPIO_OE |= (1 << DISPLAY_CD); 
    *DISPLAY_RESET_PIN_CNTR = 5;
    *DISPLAY_CD_PIN_CNTR = 5;
    // set reset high
    *(GPIO_OUT + 1) = (1 << DISPLAY_RESET);

    // configure control register 0: 8-bit data, 199 as SCR resuting in a initial clock rate of 300 kHz at 120 MHz
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (SCK_SDCARD_INIT << SPI_SSPCR0_SCR_LSB);
    // configure clock divider
    *SSPCPSR = 2;
    // configure control register 1: enable by setting synchronous operation
    *SSPCR1 = 1 << SPI_SSPCR1_SSE_LSB;
}


void sendSdCardDummyBytes(uint16_t cnt,uint8_t targetbyte)
{
    uint8_t dummy = 0;
    waitUntilBusReady();
    csDisableDisplay();
    csEnableSDCard();
    if (targetbyte > 0)
    {
        uint16_t c=0;
        while(c<cnt && dummy != targetbyte)
        {
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            dummy = *SSPDR & 0xFF;
            *SSPDR = 0xFF;
            c++;
        }
    }
    else
    {
        for(uint16_t c=0;c<cnt;c++)
        {
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            dummy = *SSPDR & 0xFF;
            *SSPDR = 0xFF;
            c++;
        }
    }
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

}

uint8_t sendSdCardCommand(uint8_t* cmd,uint8_t* resp,uint16_t len)
{

    uint8_t c = 0;
    uint8_t retval,returncode=0;
    uint16_t retcnt=0;
    waitUntilBusReady();
    sendSdCardDummyBytes(2,0);
    for(c=0;c<6;c++) // send all commands at once
    {
        *SSPDR = cmd[c];
    }
    c=0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ) // wait until command is sent, read back dummy bytes
    {
        if((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))==(1 << SPI_SSPSR_RNE_LSB))
        {
            retval = *SSPDR & 0xFF;
            c++;
        }
    }
    while((*SSPSR & (1 << SPI_SSPSR_RNE_LSB))==(1 << SPI_SSPSR_RNE_LSB)) // read back remaining dummy bytes
    {
        retval = *SSPDR & 0xFF;
        c++;
    }
    // wait for response
    c = 0;
    while(retcnt < len && c < 0x40)
    {
        *SSPDR = 0xFF;
        waitUntilBusReady();
        retval = *SSPDR;
        if ((retcnt==0 && (retval & 0x80))==0 || retcnt > 0)
        {
            *(resp + retcnt++) = retval;
        }
        c++;
        if (c == 0x40)
        {
            returncode = ERROR_TIMEOUT;
        }
    }
    waitUntilBusReady();
    sendSdCardDummyBytes(2,0);
    return returncode;
}

uint8_t initSdCard()
{
    uint8_t cmd[6];
    uint8_t resp[5];
    uint8_t retcode;
    waitUntilBusReady();
    csDisableDisplay();
    csEnableSDCard();
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (SCK_SDCARD_INIT << SPI_SSPCR0_SCR_LSB);
    // send 10 dummy bytes
    sendSdCardDummyBytes(10,0);

    // send command 0
    cmd[0] = 0x40;
    cmd[1] = 0x0;
    cmd[2] = 0x0;
    cmd[3] = 0x0;
    cmd[4] = 0x0;
    cmd[5] = 0x95;
    retcode = sendSdCardCommand(cmd,resp,1);
    if (retcode != 0)
    {
        return retcode;
    }
    if (resp[0] != (1 << R1_IDLE))
    {
        return ERROR_CARD_UNRESPONSIVE; // the card is probably unresponsive
    }

    // send command 8
    cmd[0] = 0x48;
    cmd[1] = 0x0;
    cmd[2] = 0x0;
    cmd[3] = 0x1;
    cmd[4] = 0xAA;
    cmd[5] = 0x87;
    retcode = sendSdCardCommand(cmd,resp,5);
    if (retcode != 0)
    {
        return retcode;
    }
    if (resp[0] == SD_CARD_VERSION_2)
    {
        for (uint8_t c2=1;c2<5;c2++)
        {
            if(cmd[c2] != resp[c2])
            {
                return ERROR_V2_CMD8_RESPONSE;
            }
        }

        while((resp[0] & 0x01) == 0x01)
        {
            // send CMD55
            cmd[0] = 0x40 + 55;
            cmd[1] = 0x0;
            cmd[2] = 0x0;
            cmd[3] = 0x0;
            cmd[4] = 0x0;
            cmd[5] = 0xFF;
            retcode = sendSdCardCommand(cmd,resp,1);
            if (retcode != 0)
            {
                return retcode;
            }
            // send command CMD41
            cmd[0] = 0x40 + 41;
            cmd[1] = 0x40;
            cmd[2] = 0x0;
            cmd[3] = 0x0;
            cmd[4] = 0x0;
            cmd[5] = 0xFF;
            retcode = sendSdCardCommand(cmd,resp,5);
            if (retcode != 0)
            {
                return retcode;
            }
        }
    }
    else{ // for sd card version 1 try CMD1 only
        // send command CMD1
        cmd[0] = 0x40 + 1;
        cmd[1] = 0x40;
        cmd[2] = 0x0;
        cmd[3] = 0x0;
        cmd[4] = 0x0;
        cmd[5] = 0xFF;
        retcode = sendSdCardCommand(cmd,resp,1);
        if (retcode != 0)
        {
            return retcode;
        }
    }

    // send CMD58
    cmd[0] = 0x40 + 58;
    cmd[1] = 0x0;
    cmd[2] = 0x0;
    cmd[3] = 0x0;
    cmd[4] = 0x0;
    cmd[5] = 0xFF;
    retcode = sendSdCardCommand(cmd,resp,5);
    if (retcode != 0)
    {
        return retcode;
    }
    if ((resp[0] & (1 << R1_ILLEGAL_COMMAND)) == (1 << R1_ILLEGAL_COMMAND))
    {
        return ERROR_ILLEGAL_COMMAND;
    }

    if ((resp[0] & (1 << R1_ILLEGAL_COMMAND)) == (1 << R1_ILLEGAL_COMMAND))
    {
        return 3;
    }
    // set block size to 512 bytes
    // send CMD16
    cmd[0] = 0x40 + 16;
    cmd[1] = 0x0;
    cmd[2] = 0x0;
    cmd[3] = 0x2;
    cmd[4] = 0x0;
    cmd[5] = 0xFF;
    retcode = sendSdCardCommand(cmd,resp,1);
    if (retcode != 0)
    {
        return retcode;
    }
    if ((resp[0] & (1 << R1_ILLEGAL_COMMAND)) == (1 << R1_ILLEGAL_COMMAND))
    {
        return ERROR_ILLEGAL_COMMAND;
    }

    return 0;
}

void sendDisplayCommand(uint8_t cmd,const uint8_t * data,uint32_t dataLen)
{
    waitUntilBusReady();
    csDisableSDCard();
    csEnableDisplay();
    setSckDisplay();
    uint32_t cnt=0;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = cmd;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    if (dataLen > 0)
    {
        *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
        for(cnt=0;cnt<dataLen;cnt++)
        {
            *SSPDR = *(data+cnt);
            while ((*SSPSR & (1 << SPI_SSPSR_TNF_LSB))==0); 
        }
    }
}

uint8_t readSector(uint8_t* sect, uint32_t address)
{
    uint8_t cmd[6];
    uint8_t retcode;
    uint16_t cSect;
    uint32_t c;
    waitUntilBusReady();
    csDisableDisplay();
    csEnableSDCard();    
    setSckSdCard();
    // send CMD17 (read one block)
    cmd[0]=17 + 0x40;
    cmd[1] = (address>>24) & 0xFF;
    cmd[2] = (address>>16) & 0xFF;
    cmd[3] = (address>>8) & 0xFF;
    cmd[4] = address & 0xFF;
    cmd[5] = 0xFF;
    retcode = sendSdCardCommand(cmd,sect,1);
    if (retcode != 0)
    {
        return ERROR_TIMEOUT;
    }
    if(sect[0]!= 0x0)
    {
        return ERROR_READ_FAILURE;
    }
    

    c=0;
    cSect = 0;
    uint8_t dataBeginMarker=0;
    while(cSect < 512 && c < SD_CARD_READ_TIMEOUT)
    {
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        if (dataBeginMarker != 0xFE)
        {
            dataBeginMarker = *SSPDR & 0xFF;
        } else
        {
            *(sect + cSect++) = *SSPDR & 0xFF;
        }
        *SSPDR = 0xFF;
        c++;
    }
    *SSPDR = 0xFF;
    *SSPDR = 0xFF;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    if (c == SD_CARD_READ_TIMEOUT)
    {
        return ERROR_READ_TIMEOUT;
    }
    else
    {
        return 0;
    }
}


uint8_t writeSector(uint8_t* sect, uint32_t address)
{
    uint8_t cmd[6];
    uint8_t retcode;
    uint16_t cSect;
    uint8_t resp[2];
    uint16_t c=0;
    uint8_t rCode;
    waitUntilBusReady();
    csDisableDisplay();
    csEnableSDCard();    
    setSckSdCard();
    // send CMD24 (write one block)
    cmd[0]=24 + 0x40;
    cmd[1] = (address>>24) & 0xFF;
    cmd[2] = (address>>16) & 0xFF;
    cmd[3] = (address>>8) & 0xFF;
    cmd[4] = address & 0xFF;
    cmd[5] = 0xFF;
    retcode = sendSdCardCommand(cmd,resp,1);
    if (retcode != 0)
    {
        return ERROR_TIMEOUT;
    }
    if(resp[0]!= 0x0)
    {
        return ERROR_READ_FAILURE;
    }
    
    *SSPDR = 0xFF;
    *SSPDR = 0xFF;
    cSect = 0;
    *SSPDR = 0xFE;
    while(cSect < 512)
    {
        rCode=*SSPDR;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        *SSPDR = *(sect + cSect++);
    }
    
    while(c < SD_CARD_READ_TIMEOUT)
    {
        *SSPDR = 0xFF;
        while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
        rCode = *SSPDR & 0xFF;
        if (rCode != 0xFF)
        {
            break;
        }
        c++;
    }
    c=0;
    if ((rCode & 0x1F) == 0x05)
    {
        while(c < SD_CARD_WRITE_TIMEOUT)
        {
            *SSPDR = 0xFF;
            while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
            rCode = (*SSPDR & 0xFF);
            if(rCode != 0x00)
            {
                break;
            }
        }
        c++;
    }

    *SSPDR = 0xFF;
    *SSPDR = 0xFF;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    return 0;
}


void setBacklight(uint8_t brightness)
{
    *PWM_CH0_CC = brightness << 8; 
}

void initDisplay()
{
    waitUntilBusReady();
    csDisableSDCard();
    csEnableDisplay();
    *SSPCR0 = (0x7 << SPI_SSPCR0_DSS_LSB) | (SCK_DISPLAY_SLOW << SPI_SSPCR0_SCR_LSB);

    // pull reset down
    *(GPIO_OUT + 2) = (1 << DISPLAY_RESET);
    waitSysticks(2);
    // pull up reset
    *(GPIO_OUT + 1) = (1 << DISPLAY_RESET);
    // wait 120ms
    waitSysticks(12);

    //x-y exchange, change y order
    
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x36;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
    *SSPDR = (1 << ST7735_MADCTL_MV) | (1 << ST7735_MADCTL_MY);
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // software reset
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x01;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // wait 120ms
    waitSysticks(12);

    // sleep out
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x11;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // wait 120ms
    waitSysticks(12);

    // color mode 16bit
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x3A;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
    *SSPDR = 0x5;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );     

    // display on
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x29;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 

    // use pwm channel 0 for backlight brightness control
    *PWM_CH0_TOP = 0xFFFF;
    *PWM_CH0_CC = 0x7FFF; 
    *PWM_CH0_CSR |= (1 << PWM_CH0_CSR_EN_LSB);
    *GPIO_OE |= (1 << DISPLAY_BACKLIGHT);
    *DISPLAY_BACKLIGHT_PIN_CNTR = 4;

    // blank screen
    blankScreen();
}


uint8_t blankScreen()
{
    uint8_t r = 30 << 3;
    uint8_t g = 00 << 2;
    uint8_t b = 10 << 3;
    waitUntilBusReady();
    csDisableSDCard();
    csEnableDisplay();
    setSckDisplay();
    // CASET
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x2A;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
    *SSPDR = 0x0;
    *SSPDR = 0x0;
    *SSPDR = 0x0;
    *SSPDR = 0x9F;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );  

    // RASET
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x2B;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
    *SSPDR = 0x0;
    *SSPDR = 0x0;
    *SSPDR = 0x0;
    *SSPDR = 0x7F;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );      

    // RAMWR
    *(GPIO_OUT + 2) = (1 << DISPLAY_CD);
    *SSPDR = 0x2C;
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) ); 
    *(GPIO_OUT + 1) = (1 << DISPLAY_CD);
    uint16_t rCnt = 0;
    uint16_t cCnt = 0;
    uint8_t cbyte1, cbyte2;
    while(cCnt < 128)
    {
        rCnt=0;
        while (rCnt < 160)
        {
            r = rCnt;
            b = cCnt;
            cbyte1 = (r & 0xF8) | ((g >> 5) & 0x7);
            cbyte2 = ((g & 0x7) << 5) | ((b & 0xF8) >> 3);
            while ((*SSPSR & (1 << SPI_SSPSR_TNF_LSB))==0); 
            *SSPDR = cbyte1;
            while ((*SSPSR & (1 << SPI_SSPSR_TNF_LSB))==0); 
            *SSPDR = cbyte2;
            rCnt++;
        }
        cCnt++;
    }
    return 0;
}

void displayOff()
{
    waitUntilBusReady();
    csDisableSDCard();
    csEnableDisplay();
    setSckDisplay();
    sendDisplayCommand(0x28,(uint8_t*)0,0);
}

void displayOn()
{
    waitUntilBusReady();
    csDisableSDCard();
    csEnableDisplay();
    setSckDisplay();
    sendDisplayCommand(0x29,(uint8_t*)0,0);
}
