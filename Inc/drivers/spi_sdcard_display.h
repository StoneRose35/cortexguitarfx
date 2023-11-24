#ifndef _SPI_SDCARD_H
#define _SPI_SDCARD_H

#include <stdint.h>
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


void initSpi();
void sendSdCardDummyBytes(uint16_t cnt,uint8_t targetbyte);
uint8_t sendSdCardCommand(uint8_t* cmd,uint8_t* resp,uint16_t len);
uint8_t initSdCard();
uint8_t readSector(uint8_t*, uint32_t);
uint8_t writeSector(uint8_t*, uint32_t);
void sendDisplayCommand(uint8_t,const uint8_t *,uint32_t);

inline void csEnableDisplay()
{    
    *CS_DISPLAY_PIN_CNTR = 5;
    *(GPIO_OUT + 2) = (1 << CS_DISPLAY);
}

inline void csDisableDisplay()
{
    *CS_DISPLAY_PIN_CNTR = 5;
    *(GPIO_OUT + 1) = (1 << CS_DISPLAY);
}

inline void setSckDisplay()
{
    *SSPCR0 &= ~SPI_SSPCR0_SCR_BITS;
    *SSPCR0 |= (SCK_DISPLAY_SLOW << SPI_SSPCR0_SCR_LSB);
}

inline void setSckSdCard()
{
    *SSPCR0 &= ~SPI_SSPCR0_SCR_BITS;
    *SSPCR0 |= (SCK_SDCARD_MEDIUM << SPI_SSPCR0_SCR_LSB);
}

inline void csEnableSDCard()
{
    *CS_SDCARD_PIN_CNTR = 1;
}

inline void csDisableSDCard()
{
    *CS_SDCARD_PIN_CNTR = 5;
    *(GPIO_OUT + 1) = (1 << CS_SDCARD);
}

inline void waitUntilBusReady()
{
    while ((*SSPSR & (1 << SPI_SSPSR_BSY_LSB))==(1 << SPI_SSPSR_BSY_LSB) );
}

void initDisplay();
uint8_t blankScreen();
void setBacklight(uint8_t);
void displayOff();
void displayOn();

#define SD_CARD_VERSION_2 1
#define SD_CARD_VERSION_1 5

#define R1_IDLE 0
#define R1_ILLEGAL_COMMAND 2
#define R1_CRC_ERROR 3

// -------------------------
// -  Error Codes
// -------------------------
#define ERROR_TIMEOUT 1
#define ERROR_CARD_UNRESPONSIVE 2
#define ERROR_V2_CMD8_RESPONSE 3
#define ERROR_ILLEGAL_COMMAND 4
#define ERROR_READ_FAILURE 5
#define ERROR_WRITE_FAILURE 6
#define ERROR_READ_TIMEOUT 7

// -------------------------- 
// -  ST7735 bit definitions
// --------------------------
#define ST7735_MADCTL_MY 7
#define ST7735_MADCTL_MX 6
#define ST7735_MADCTL_MV 5
#define ST7735_MADCTL_ML 4
#define ST7735_MADCTL_RGB 3
#define ST7735_MADCTL_MH 2

#define SD_CARD_READ_TIMEOUT 100000
#define SD_CARD_WRITE_TIMEOUT 312500
#endif