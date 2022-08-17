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

#include <stdint.h>
#include "systemChoice.h"

#define N_LAMPS (1) //!< the number of neopixel the system contains 

#define FRAMERATE (30) //!< frames or lamp images per second

// settings regarding the specific neopixels used, adjust depending on the actual neopixel chip
#define WS2818_FREQ (800000) //!< frequency of the neopixel interface, is 400000  (400 kHz)for older ones and 800000 (800 kHz)for new ones
#define WS2818_SHORT (2500000) //!< 1/pulse length of the short neopixel pulse
#define WS2818_LONG  (1176470) //!< 1/pulse length of the long neopixel pulse

#ifdef HARDWARE
#ifdef STM32
#define F_SYS (64000000)
#define F_APB1 F_SYS/2
#define WS2818_CNT F_SYS/WS2818_FREQ
#define WS2818_SHORT_CNT F_SYS/WS2818_SHORT
#define WS2818_LONG_CNT F_SYS/WS2818_LONG
#define REMAINING_WAITTIME F_SYS/FRAMERATE-F_SYS/WS2818_FREQ*24*N_LAMPS

#define I2C_ADDRESS (15)
#define FLASH_PAGE_SIZE 2048
#define FLASH_PAGE_SIZE_BIT 11
#endif

#ifdef RP2040_FEATHER

#define F_XOSC (12000000)
#define F_ADC_USB (48000000)
// divider factors, calculated using clingo with clockcalculation.lp
#define POSTDIV1 6
#define POSTDIV1_USB 6
#define POSTDIV2 2
#define POSTDIV2_USB 5
#define NP_CLKDIV 15
#define FEEDBK 120
#define FEEDBK_USB 120
#define F_SYS (F_XOSC*FEEDBK/POSTDIV1/POSTDIV2)
#define F_USB (F_XOSC*FEEDBK_USB/POSTDIV1_USB/POSTDIV2_USB)
#define PIO_SM1_CNT (F_SYS/FRAMERATE-2) 

#define FLASH_PAGE_SIZE 4096 //!< size in bytes of a flash page, a flash page is the smallest eraseable flash unit
#define FLASH_PAGE_SIZE_BIT 12 //!< bit position of the flash page size, i.e. 2^FLASH_PAGE_SIZE_BIT = FLASH_PAGE_SIZE


#endif

#else
#define FLASH_PAGE_SIZE 16
#define FLASH_PAGE_SIZE_BIT 4
#endif



#define PHASE_INC (12) //!<phase increment for the hue shift function in main.c



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

/**
 * @brief a simple rgb structure, is not aligned to be streamed directly to the neopixels
 *
 */
typedef struct
{
	uint8_t r; //!< Red
	uint8_t g; //!< Green
	uint8_t b; //!< Blue
} RGB;

/**
 * @brief overlay of a potentially streamable color information with the color structure, currently unused
 * 
 */
typedef union
{
	RGB rgb; //!< the color struct
	uint32_t stream; //!< the streamable content of the color struct
} RGBStream;

#endif /* SYSTEM_H_ */
