#ifndef _USB_CDC_EDITOR_INTERFACE_H_
#define  _USB_CDC_EDITOR_INTERFACE_H_
#include "stdint.h"


typedef struct 
{
    uint16_t commandNr;
    uint16_t commandSize;
} UsbEditorCommandHeaderType;

#define USB_CMD_GET_ABOUT 0
#define USB_CMD_GET_VERSION 1
#define USB_CMD_SET_HIZ 2
#define USB_CMD_GET_SIZ 3
#define USB_CMD_SET_MIC 4
#define USB_CMD_GET_MIC 5
#define USB_CMD_GET_VOLUME 6
#define USB_CMD_SET_VOLUME 7
#define USB_CMD_GET_PROGRAMS
//#define USB_CMD_
//#define USB_CMD_
//#define USB_CMD_

#define MSG_ABOUT 0

void processUSBEditorCommand(uint8_t * cmd);
#endif