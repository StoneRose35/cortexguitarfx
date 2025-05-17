/**
 * @file system.h
 * @author Philipp Fuerholz (fuerholz@gmx.ch)
 * @brief contains microprocessor-independent information on the lamp system
 * such as the number of lamp attached and the intended framerate
 * while the system clock frequency can be entirely defined within neopixelDriver.h
 * the framerate settings should be entirely deriveable from FRAMERATE
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "systemChoice.h"

#ifdef HARDWARE

#ifdef RP2040_FEATHER

// --------------------------------------
// - Configuration of clock frequencies -
// --------------------------------------
#define F_XOSC (12000000)
#define F_ADC_USB (48000000)
// system and usb clocks
// divider factors, calculated using helper_scripts.py
#define POSTDIV1 3
#define POSTDIV1_USB 6
#define POSTDIV2 2
#define POSTDIV2_USB 5
#define NP_CLKDIV 15
#define FEEDBK 100
#define FEEDBK_USB 120
#define F_SYS (F_XOSC*FEEDBK/POSTDIV1/POSTDIV2)
#define F_USB (F_XOSC*FEEDBK_USB/POSTDIV1_USB/POSTDIV2_USB)
// flash access clock
#define PICO_FLASH_SPI_CLKDIV 2 // SSI Clock speed is F_SYS/PICO_FLASH_SPI_CLKDIV, less than 133MHz according to V25Q16JVU datasheet
                                // must be an even value
// spi clock_division
#define SPI_SCR 19
#define SPI_SSPCPSR 2
// i2c high and low cycle counts
#define I2C_SS_HCNT 800
#define I2C_SS_LCNT 940



#define FLASH_PAGE_SIZE 4096 //!< size in bytes of a flash page, a flash page is the smallest eraseable flash unit
#define FLASH_PAGE_SIZE_BIT 12 //!< bit position of the flash page size, i.e. 2^FLASH_PAGE_SIZE_BIT = FLASH_PAGE_SIZE


#endif

#else
#define FLASH_PAGE_SIZE 16
#define FLASH_PAGE_SIZE_BIT 4
#endif


#define TASK_USB_CONSOLE_RX 0 //!< bit position of the task bit array, set: the task for handling the usb console on receiving characters should be called
#define TASK_BT_CONSOLE_RX 1 //!< bit position of the task bit array, set: the task for handling the bluetooth console on receiving characters should be called
#define TASK_USB_CONSOLE_TX 2 //!< bit position of the task bit array, set: the task for handling the usb console on sending characters should be called
#define TASK_BT_CONSOLE_TX 3 //!< bit position of the task bit array, set: the task for handling the bluetooth console on sending characters should be called
#define TASK_PROCESS_AUDIO 4 //<! bit position of the task bit array, set: one half of the audio double buffer has been written completely and is available for processing
#define TASK_PROCESS_AUDIO_INPUT 5 //<! audio input buffer has been filled, process it
#define TASK_UPDATE_POTENTIOMETER_VALUES 6  //<! potentiometer values have been read --> update
#define TASK_UPDATE_AUDIO_UI 7
#define CONTEXT_USB 0 //!<bit position: if set output of prints is routed to the USB console
#define CONTEXT_BT 1 //!<bit position: if set output of prints is routed to the bluetooth console

#define FLASH_HEADER_SIZE 32 //!< size in bytes of a non-overwriteable area at the beginning of the Flash section


#endif /* SYSTEM_H_ */
