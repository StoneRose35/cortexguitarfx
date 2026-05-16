
#include "stdint.h"
#include "usb/usb_cdc_editor_interface.h"
#include "usb/usb_cdc.h"
#include "gen/version.h"
#include "stringFunctions.h"

void processUSBEditorCommand(uint8_t * cmd)
{
    char strbfr[256];
    uint16_t idx = 0;
    UsbEditorCommandHeaderType* header = (UsbEditorCommandHeaderType*)cmd;
    switch(header->commandNr)
    {
        case USB_CMD_GET_ABOUT:
            *strbfr = 0;
            idx += appendToString(strbfr+idx,"About PiPicoFX\r\n");
            idx += appendToString(strbfr + idx,PI_PICO_FX_VERSION_NR);
            idx += appendToString(strbfr + idx,"\r\n");
            idx += appendToString(strbfr + idx,PI_PICO_FX_MCU_BOARD);
            idx += appendToString(strbfr + idx,"\r\nbuilt\r\n");            
            idx += appendToString(strbfr + idx,PI_PICO_FX_BUILD_DATE);
            idx += appendToString(strbfr + idx,"\r\n      ");     
            idx += appendToString(strbfr + idx,PI_PICO_FX_BUILD_TIME);    
            sendOverUsb((uint8_t*)strbfr,idx,0);
            break;
        default:
            break;
    }
}