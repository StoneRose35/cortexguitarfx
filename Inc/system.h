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


#ifdef HARDWARE



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
#define TASK_UPDATE_AUDIO_UI 7 // update the screen content: rewrite framebuffer and issue a display driver update
#define TASK_FLASH_QSPI 8 // flash the qspi content
#define TASK_DISPLAY_NEXT_LINE 9 //issued by the display driver: next line content should be transferred from dma
#define TASK_I2C_DATA_RECEIVED 10 // I2C data has been received and is ready to be processed
#define TASK_PREPARE_FOR_DFU 11 // prepare the system for dfu, i.e. shut down interrupt and any possibilities to access flash or qspi code
#define TASK_MANIFEST_DFU 12 // DFU manifestation phase

#define CONTEXT_USB 0 //!<bit position: if set output of prints is routed to the USB console
#define CONTEXT_BT 1 //!<bit position: if set output of prints is routed to the bluetooth console

#define FLASH_HEADER_SIZE 32 //!< size in bytes of a non-overwriteable area at the beginning of the Flash section



#endif /* SYSTEM_H_ */
