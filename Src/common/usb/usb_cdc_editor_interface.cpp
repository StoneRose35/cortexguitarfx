
#include "stdint.h"
extern "C" {
#include "usb/usb_vendor_specific_dfu_capable.h"
#include "gen/version.h"
#include "stringFunctions.h"
#include "pipicofx/pipicofxui.h"
#include "memoryRegions.h"
#include "pcm3060.h"
#include "drivers/24lc128.h"
#include "timer.h"
}

#include "usb/usb_cdc_editor_interface.hpp"
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/uiLevel3_preset.hpp"

using namespace PiPicoFX;
extern PiPicoFXUiType ui;
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern MultiAudioProcessor audioProcessor;
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];
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
        case USB_CMD_GET_PARAMETER_NAMES:
            processGetParameterNamesCmd(*(cmd+4));
            break;
        case USB_CMD_GET_CURRENT_BANK_PRESET_NR:
            processGetCurrentBankAndPresetNr();
            break;
        case USB_CMD_GET_PRESET:
            processGetPreset(cmd+4);
            break;
        case USB_CMD_SET_PARAMETER:
            processSetParameter(cmd+4);
            break;
        case USB_CMD_LOAD_PRESET:
            processLoadPreset(*(cmd+4));
            break;
        case USB_CMD_SET_FX_PROGRAM:
            processSetFxProgram(cmd+4);
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
    usbVendorSpecificSendData((uint8_t*)strbfr,4,1);
    usbVendorSpecificSendData((uint8_t*)strbfr,idx,0);
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
        prog = loadProgramWithoutSetup(c);
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
    // double package, header first, then full message for the client to understand
    usbVendorSpecificSendData((uint8_t*)strbfr,4,1);
    usbVendorSpecificSendData((uint8_t*)strbfr,idx,0);
}


__QSPI_CODE
void processGetParameterNamesCmd(uint8_t programIdx)
{
    uint16_t idx=0;
    uint8_t nameIdx = 0;
    uint8_t response[256];
    FxProgram * prog;
    const char * paramName;
    prog = loadProgramWithoutSetup(programIdx);
    
    *((uint16_t*)(response+idx)) = MSG_PARAMETER_NAMES;
    response[4]=programIdx;
    idx=5;
    for (uint8_t q=0;q<prog->getParameterCount();q++)
    {
        paramName = prog->getParameter(q)->getParameterName();
        while(*(paramName + nameIdx) != 0)
        {
            *(response+idx++)=*(paramName+nameIdx++);
        }
        *(response+idx++)=0;
        nameIdx=0;
    }
    delete prog;
    *((uint16_t*)(response+2))=idx;
    usbVendorSpecificSendData((uint8_t*)response,4,1);
    usbVendorSpecificSendData((uint8_t*)response,idx,0);
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
    usbVendorSpecificSendData(responseBfr,4,1);
    usbVendorSpecificSendData(responseBfr,6,0);
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
    usbVendorSpecificSendData(responseBfr,4,1);
    usbVendorSpecificSendData(responseBfr,6,0);
}

__QSPI_CODE
void processGetCurrentBankAndPresetNr()
{
    uint8_t responseBfr[6];
    responseBfr[0]=MSG_BANK_AND_PRESET_NR;
    responseBfr[1]=0;
    responseBfr[2]=6;
    responseBfr[3]=0;
    responseBfr[4]=currentBank;
    responseBfr[5]=currentPreset;
    usbVendorSpecificSendData(responseBfr,4,1);
    usbVendorSpecificSendData(responseBfr,6,0);
}

__QSPI_CODE
void processGetPreset(uint8_t*data)
{
    uint16_t idx=4;
    uint8_t stringIndex=0;
    uint8_t responseBfr[512];
    char parameterDisplayValueBfr[64];
    FxPresetType preset;
    FxProgram * prog;
    uint8_t programsStati = 0;
    if ((*(data))*3 + *(data+1) == currentBank*3 + currentPreset)
    {
        // load to get name, then transfer current parameters
        if (loadPreset(&preset,(*(data))*3 + *(data+1))!=0)
        {
            generateEmptyPreset(&preset,*data,*(data+1));
        }
        parametersToPreset(&preset,&audioProcessor);
        if (((FxProgram*)audioProcessor.getFxProgram(0))!= 0)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
            {
                programsStati &= ~0x3;
                programsStati |= 1;
            }
            else if (((FxProgram*)audioProcessor.getFxProgram(0))->isFrozen())
            {
                programsStati &= ~0x3;
                programsStati |= 2;
            }
        }
        if (((FxProgram*)audioProcessor.getFxProgram(1))!= 0)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
            {
                programsStati &= ~(0x3 << 2);
                programsStati |= 1 << 2;
            }
            else if (((FxProgram*)audioProcessor.getFxProgram(1))->isFrozen())
            {
                programsStati &= ~(0x3 << 2);
                programsStati |= 2 << 2;
            }
        }
        if (((FxProgram*)audioProcessor.getFxProgram(2))!= 0)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
            {
                programsStati &= ~(0x3 << 4);
                programsStati |= 1 << 4;
            }
            else if (((FxProgram*)audioProcessor.getFxProgram(2))->isFrozen())
            {
                programsStati &= ~(0x3 << 4);
                programsStati |= 2 << 4;
            }
        }
    }
    else
    { 
        if (loadPreset(&preset,(*(data))*3 + *(data+1))!=0)
        {
            generateEmptyPreset(&preset,*data,*(data+1));
        }
        if (preset.programNrA != 0x3F)
        {
            programsStati |= 1;
        }
        if (preset.programNrB != 0x3F)
        {
            programsStati |= 1 << 2;
        }
        if (preset.programNrC != 0x3F)
        {
            programsStati |= 1 << 4;
        }
    }
    responseBfr[0]=MSG_PRESET;
    responseBfr[1]=0;
    responseBfr[idx++]=*data;
    responseBfr[idx++]=*(data+1) | (preset.routing << 2);
    if (preset.programNrA != 0x3F)
    responseBfr[idx++] = programsStati;
    while(*(preset.name + stringIndex)!=0)
    {
        *(responseBfr+idx++) = *(preset.name + stringIndex++);
    }
    *(responseBfr+idx++)=0;
    *(responseBfr+idx++)=preset.programNrA;
    if (preset.programNrA != 0x3F)
    {
        prog = loadProgramWithoutSetup(preset.programNrA);
        for (uint8_t c=0;c<prog->getParameterCount();c++)
        {
            *((uint16_t*)(responseBfr + idx))=preset.parametersA[c];
            idx+=2;
            *parameterDisplayValueBfr=0;
            prog->getParameter(c)->parameterCallback(preset.parametersA[c]);
            prog->getParameter(c)->parameterDisplay(parameterDisplayValueBfr);
            uint8_t displayNameIdx=0;
            while(*(parameterDisplayValueBfr + displayNameIdx)!= 0)
            {
                *(responseBfr + idx++) = *(parameterDisplayValueBfr + displayNameIdx++);
            }
            *(responseBfr + idx++)=0;
        } 
        delete prog;
    }
    *(responseBfr+idx++)=preset.programNrB;
    if (preset.programNrB != 0x3F)
    {
        prog = loadProgramWithoutSetup(preset.programNrB);
        for (uint8_t c=0;c<prog->getParameterCount();c++)
        {
            *((uint16_t*)(responseBfr + idx))=preset.parametersB[c];
            idx+=2;
            *parameterDisplayValueBfr=0;
            prog->getParameter(c)->parameterCallback(preset.parametersB[c]);
            prog->getParameter(c)->parameterDisplay(parameterDisplayValueBfr);
            uint8_t displayNameIdx=0;
            while(*(parameterDisplayValueBfr + displayNameIdx)!= 0)
            {
                *(responseBfr + idx++) = *(parameterDisplayValueBfr + displayNameIdx++);
            }
            *(responseBfr + idx++)=0;
        } 
        delete prog;
    }
    *(responseBfr+idx++)=preset.programNrC;
    if (preset.programNrC != 0x3F)
    {
        prog = loadProgramWithoutSetup(preset.programNrC);
        for (uint8_t c=0;c<prog->getParameterCount();c++)
        {
            *((uint16_t*)(responseBfr + idx))=preset.parametersC[c];
            idx+=2;
            *parameterDisplayValueBfr=0;
            prog->getParameter(c)->parameterCallback(preset.parametersC[c]);
            prog->getParameter(c)->parameterDisplay(parameterDisplayValueBfr);
            uint8_t displayNameIdx=0;
            while(*(parameterDisplayValueBfr + displayNameIdx)!= 0)
            {
                *(responseBfr + idx++) = *(parameterDisplayValueBfr + displayNameIdx++);
            }
            *(responseBfr + idx++)=0;
        } 
        delete prog;
    }
    *((uint16_t*)(responseBfr+2))=idx;
    usbVendorSpecificSendData((uint8_t*)responseBfr,4,1);
    usbVendorSpecificSendData((uint8_t*)responseBfr,idx,0);

}

__QSPI_CODE
void processSetParameter(uint8_t*data)
{
    uint16_t stringIndex=6;
    uint8_t responseBfr[512];

    uint8_t effectIndex = data[0];
    uint8_t parameterIndex = data[1];
    uint16_t paramValue = *((uint16_t*)(data+2));
    if ((FxProgram*)audioProcessor.getFxProgram(effectIndex)!=0)
    {
        ((FxProgram*)audioProcessor.getFxProgram(effectIndex))
        ->getParameter(parameterIndex)
        ->parameterCallback(paramValue);
    }
    ((FxProgram*)audioProcessor.getFxProgram(effectIndex))
        ->getParameter(parameterIndex)->parameterDisplay((char*)responseBfr+6);
    responseBfr[0]=MSG_PARAMETER_VALUE;
    responseBfr[1]=0;
    responseBfr[4]=effectIndex;
    responseBfr[5]=parameterIndex;
    while (*(responseBfr + stringIndex) != 0)
    {
        stringIndex++;
    }
    stringIndex++;
    *((uint16_t*)(responseBfr+2))=stringIndex;
    usbVendorSpecificSendData((uint8_t*)responseBfr,4,1);
    usbVendorSpecificSendData((uint8_t*)responseBfr,stringIndex,0);
}

__QSPI_CODE
void processLoadPreset(uint8_t data)
{
    currentPreset = data; 
    setPreset();
    
}

__QSPI_CODE
void processSetFxProgram(uint8_t*data)
{
    if (programChangeState == 0)
    {
        programsToInitialize[*data]=*(data+1) | 0x80; // set fx program at defined position, reload values from preset
        programChangeState=1;
    }

}