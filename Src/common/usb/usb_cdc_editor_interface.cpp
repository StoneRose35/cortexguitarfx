
#include "stdint.h"
extern "C" {
#include "usb/usb_cdc.h"
#include "gen/version.h"
#include "stringFunctions.h"
#include "memoryRegions.h"
#include "pcm3060.h"
}

#include "usb/usb_cdc_editor_interface.hpp"
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/FxProgram.hpp"

using namespace PiPicoFX;
__QSPI_CODE
void processUSBEditorCommand(uint8_t * cmd)
{
    UsbEditorCommandHeaderType* header = (UsbEditorCommandHeaderType*)cmd;
    switch(header->commandNr)
    {
        case USB_CMD_GET_ABOUT:
            processGetAboutCmd();
            break;
        case USB_CMD_GET_PROGRAMS:
            processGetProgramsCmd();
            break;
        case USB_CMD_GET_INPUTS_AND_MASTER_VOLUME:
            processInputStateAndMasterVolume();
            break;
        case USB_CMD_SET_INPUTS_AND_MASTER_VOLUME:
            processSetInputStateAndMasterVolume(cmd+4);
            break;
        default:
            break;
    }
}

__QSPI_CODE
void processGetAboutCmd()
{
    char strbfr[256];
    uint16_t idx = 0;
    *((uint16_t*)strbfr) = MSG_ABOUT;
    *(strbfr+4)=0;
    idx=4;
    idx += appendToString(strbfr+idx,"About PiPicoFX\r\n");
    idx += appendToString(strbfr + idx,PI_PICO_FX_VERSION_NR);
    idx += appendToString(strbfr + idx,"\r\n");
    idx += appendToString(strbfr + idx,PI_PICO_FX_MCU_BOARD);
    idx += appendToString(strbfr + idx,"\r\nbuilt\r\n");            
    idx += appendToString(strbfr + idx,PI_PICO_FX_BUILD_DATE);
    idx += appendToString(strbfr + idx,"\r\n      ");     
    idx += appendToString(strbfr + idx,PI_PICO_FX_BUILD_TIME);    
    *((uint16_t*)(strbfr+2))=idx;
    sendOverUsb((uint8_t*)strbfr,idx,0);
}

__QSPI_CODE
void processGetProgramsCmd()
{
    char strbfr[512];
    uint16_t idx = 0;
    uint8_t programProperties=0;
    uint8_t nameIdx=0;
    FxProgram * prog;
    const char * namePtr;
    *((uint16_t*)strbfr) = MSG_PROGRAMS;
    *(strbfr+4)=N_FX_PROGRAMS;
    idx=5;
    for (uint8_t c=0;c<N_FX_PROGRAMS;c++)
    {
        prog = loadProgram(c);
        programProperties = (prog->isFreezable() << 7) | (prog->getParameterCount());
        *(strbfr + idx++)=(char)programProperties;
        namePtr = prog->getName();
        nameIdx = 0;
        while(*(namePtr+nameIdx) != 0)
        {
            *(strbfr+idx++)=*(namePtr+nameIdx++);
        }
        *(strbfr + idx++) = 0;
        delete prog;
    }
    *((uint16_t*)(strbfr+2))=idx;
    sendOverUsb((uint8_t*)strbfr,idx,0);
}

__QSPI_CODE
void processInputStateAndMasterVolume()
{
    uint8_t responseBfr[6];
    // bit 1 is Mic, bit 0 is HiZ
    uint8_t regbfr = pcm3060GetInputState();
    uint16_t vol = pcm3060GetOutputVolume();
    responseBfr[0]=MSG_INPUTS_AND_MASTER_VOLUME;
    responseBfr[1]=0;
    responseBfr[2]=6;
    responseBfr[3]=0;
    responseBfr[4]= (regbfr & 0x3);
    responseBfr[5] = vol & 0xFF;
    sendOverUsb(responseBfr,6,0);
}

__QSPI_CODE
void processSetInputStateAndMasterVolume(uint8_t* data)
{
    uint8_t responseBfr[6];
    pcm3060SetInputState(PCM3060_CHANNEL_LEFT,(data[0] & 2)>>1);
    pcm3060SetInputState(PCM3060_CHANNEL_RIGHT,data[0] & 1);
    pcm3060SetOutputVolume(PCM3060_CHANNEL_BOTH,data[1]);
    responseBfr[0]=MSG_INPUTS_AND_MASTER_VOLUME;
    responseBfr[1]=0;
    responseBfr[2]=6;
    responseBfr[3]=0;
    responseBfr[4]= (data[0] & 0x3);
    responseBfr[5] = data[1] & 0xFF;
    sendOverUsb(responseBfr,6,0);
}