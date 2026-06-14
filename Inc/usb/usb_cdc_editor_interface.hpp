#ifndef _USB_CDC_EDITOR_INTERFACE_H_
#define  _USB_CDC_EDITOR_INTERFACE_H_
#include "stdint.h"


typedef struct 
{
    uint16_t commandNr;
    uint16_t commandSize;
} UsbEditorCommandHeaderType;

#define USB_CMD_GET_ABOUT 0
#define USB_CMD_GET_PARAMETER_NAMES 2
#define USB_CMD_GET_INPUTS_AND_MASTER_VOLUME 3
#define USB_CMD_SET_INPUTS_AND_MASTER_VOLUME 4
#define USB_CMD_GET_CURRENT_BANK_PRESET_NR 5
#define USB_CMD_GET_PRESET 6
#define USB_CMD_SET_PARAMETER 7
#define USB_CMD_GET_PROGRAMS 8
#define USB_CMD_LOAD_PRESET 9


#define MSG_ABOUT 0
#define MSG_PROGRAMS 1
#define MSG_PARAMETER_NAMES 2
#define MSG_INPUTS_AND_MASTER_VOLUME 3
#define MSG_BANK_AND_PRESET_NR 5
#define MSG_PRESET 6
#define MSG_PARAMETER_VALUE 7

void processUSBEditorCommand(uint8_t * cmd);
void processGetAboutCmd();
void processGetProgramsCmd();
void processInputStateAndMasterVolume();
void processSetInputStateAndMasterVolume(uint8_t*);
void processGetParameterNamesCmd(uint8_t);
void processGetCurrentBankAndPresetNr();
void processGetPreset(uint8_t*);
void processSetParameter(uint8_t*);
void processLoadPreset(uint8_t);
#endif