/**
 * @file system.h
 * @author Philipp Fuerholz
 * @brief 
 * @version 0.1
 * @date 2022-12-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>
#include "systemChoice.h"



#ifdef HARDWARE


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


#endif /* SYSTEM_H_ */
