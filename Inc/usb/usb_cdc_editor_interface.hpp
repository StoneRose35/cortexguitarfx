#ifndef _USB_CDC_EDITOR_INTERFACE_H_
#define  _USB_CDC_EDITOR_INTERFACE_H_
#include "stdint.h"


typedef struct 
{
    uint16_t commandNr;
    uint16_t commandSize;
} UsbEditorCommandHeaderType;

#define USB_CMD_GET_ABOUT 0
#define USB_CMD_GET_INPUTS_AND_MASTER_VOLUME 3
#define USB_CMD_SET_INPUTS_AND_MASTER_VOLUME 4
#define USB_CMD_GET_PROGRAMS 8
//#define USB_CMD_
//#define USB_CMD_
//#define USB_CMD_

#define MSG_ABOUT 0
#define MSG_PROGRAMS 1
#define MSG_INPUTS_AND_MASTER_VOLUME 3

void processUSBEditorCommand(uint8_t * cmd);
void processGetAboutCmd();
void processGetProgramsCmd();
void processInputStateAndMasterVolume();
void processSetInputStateAndMasterVolume(uint8_t*);
#endif