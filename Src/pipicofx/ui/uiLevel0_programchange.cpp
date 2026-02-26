extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "drivers/systick.h"
}
#include "pipicofx/FxProgramLoader.hpp"

static void knob0Callback(uint16_t val);
static void knob1Callback(uint16_t val);
static void knob2Callback(uint16_t val);
extern PiPicoFXUiType ui;
uint8_t locksymbol[5]={0b01111000,0b01111110,0b01111001,0b01111110,0b01111000 };
BwImageTypeConst lock=
{
    .data = locksymbol,
    .sx=5,
    .sy=8,
    .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES
};
extern volatile uint8_t programToInitialize;
extern volatile uint8_t programChangeState;
extern volatile uint8_t consumeEnterReleased;
extern const uint8_t stompswitch_progs[];
extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static volatile uint32_t longPressCnt=0;
static volatile uint8_t enterState=0;
static void create()
{
    char lineBuffer[24];
    BwImageType* imgBuffer = getImageBuffer();
    lock.data =locksymbol;
    clearImage(imgBuffer);
    const GFXfont * font =  getGFXFont(FREESANS9PT7B);
    drawText(0,1*14,ui.currentProgram->getName(),imgBuffer,font);
    if (ui.locked != 0)
    {
        drawImage(122,0,&lock,imgBuffer);
    }

    for (uint8_t c=0;c<ui.currentProgram->getParameterCount();c++)
    {
        if (ui.currentProgram->getParameter(c)->getControl() == 0)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P1:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,5*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 1)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P2:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,6*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 2)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P3:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,7*8,lineBuffer,imgBuffer,0);
        }                
    }
    for (uint8_t c=0;c < ui.currentProgram->getParameterCount();c++)
    {
        if (ui.currentProgram->getParameter(c)->getControl()==0)
        {
            ui.currentProgram->getParameter(c)->parameterCallback(getChannel0Value());
        }
        else if (ui.currentProgram->getParameter(c)->getControl()==1)
        {
            ui.currentProgram->getParameter(c)->parameterCallback(getChannel1Value());
        }
        else if (ui.currentProgram->getParameter(c)->getControl()==2)
        {
            ui.currentProgram->getParameter(c)->parameterCallback(getChannel2Value());
        }
    }
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    uint8_t bargraphBuffer[128];
    BwImageType bargraph={
        .data = bargraphBuffer,
        .sx = 128,
        .sy = 4,
        .type = BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize = 128
    };
    BwImageType* imgBuffer = getImageBuffer();
    
    bargraph.data = bargraphBuffer;
    // show basic display
    for (uint8_t c=0;c<128;c++)
    {
        clearPixel(c,0,&bargraph);
        clearPixel(c,3,&bargraph);
        if (c<=avgInput)
        {
            setPixel(c,1,&bargraph);
            setPixel(c,2,&bargraph);
        }
        else
        {
            clearPixel(c,1,&bargraph);
            clearPixel(c,2,&bargraph);
        }
    }
    drawImage(0,32-3*4,(BwImageTypeConst*)&bargraph,imgBuffer);

    for (uint8_t c=0;c<128;c++)
    {
        clearPixel(c,0,&bargraph);
        clearPixel(c,3,&bargraph);
        if (c<=avgOutput)
        {
            setPixel(c,1,&bargraph);
            setPixel(c,2,&bargraph);
        }
        else
        {
            clearPixel(c,1,&bargraph);
            clearPixel(c,2,&bargraph);
        }
    }
    drawImage(0,32-2*4,(BwImageTypeConst*)&bargraph,imgBuffer);


    for (uint8_t c=0;c<128;c++)
    {
        clearPixel(c,0,&bargraph);
        clearPixel(c,3,&bargraph);
        if (c<=cpuLoad)
        {
            setPixel(c,1,&bargraph);
            setPixel(c,2,&bargraph);
        }
        else
        {
            clearPixel(c,1,&bargraph);
            clearPixel(c,2,&bargraph);
        }
    }
    drawImage(0,32-1*4,(BwImageTypeConst*)&bargraph,imgBuffer);
    if (longPressCnt > 0 && getTickValue() - longPressCnt > LONGPRESS_DURATION_SYSTICKS && ui.currentProgram->isFreezable() && ui.currentProgram->isOn())
    {
        ui.currentProgram->freeze();
        longPressCnt=0;
        setStompswitchColorRaw(1 << 2);
    }
}

static inline void knobCallback(uint16_t val,uint8_t control)
{
    if (ui.locked == 0)
    {
        for (uint8_t c=0;c<ui.currentProgram->getParameterCount();c++)
        {
            if (ui.currentProgram->getParameter(c)->getControl()==control)
            {
                ui.currentProgram->getParameter(c)->parameterCallback(val);
            }
        }  
    } 
}



static void knob0Callback(uint16_t val)
{
    knobCallback(val,0);
}

static void knob1Callback(uint16_t val)
{
    knobCallback(val,1);
}

static void knob2Callback(uint16_t val)
{
    knobCallback(val,2);
}

static void enterPressedCallback()
{
    enterState = 1;
}

static void enterReleasedCallback(void) 
{
    enterState=0;
    if (consumeEnterReleased == 1)
    {
        consumeEnterReleased = 0;
        return;
    }
    if (ui.locked == 0)
    {
        uiStackPush(0);
        enterLevel1();
    }
}

static void exitCallback()
{

    if (uiStackCurrent() == 0x0)
    {
        ui.locked ^=1;
        return;
    }
    // apply current program to preset when coming from 4
    if(uiStackCurrent()==4)
    {
        presets[currentPreset].programNr = ui.currentProgramIdx;
    }
}

static void rotaryCallback(int16_t encoderDelta)
{
    if (enterState==1)
    {
        if (currentPreset == 0xFF)
        {
            currentPreset = 0;
        }
        enterState=0;
        consumeEnterReleased = 1;
        enterLevel3();
        return;
    }
    if (encoderDelta != 0 && programChangeState==0)
    {
        if (encoderDelta > 0)
        {
            encoderDelta = 1;
        }
        else
        {
            encoderDelta = -1;
        }
        ui.currentProgramIdx += encoderDelta;
        if (ui.currentProgramIdx >= N_FX_PROGRAMS && encoderDelta > 0)
        {
            ui.currentProgramIdx = N_FX_PROGRAMS-1;
        } 
        else if (ui.currentProgramIdx >= N_FX_PROGRAMS && encoderDelta < 0)
        {
            ui.currentProgramIdx = 0;
        }
        programToInitialize=ui.currentProgramIdx;
        programChangeState=1;
        setStompswitchColorRaw(0);
    }
}

/*
static void genericStompSwitchCallback(uint8_t switchNr, PiPicoFXUiType* data)
{
    currentPreset = switchNr;
    uiStackPush(data, 0);
    enterLevel3(data);
}
*/

static void stompswitch1Callback(void)
{

    ui.currentProgramIdx--;
    if (ui.currentProgramIdx >= N_FX_PROGRAMS)
    {
        ui.currentProgramIdx = 0;
    }
    programToInitialize=ui.currentProgramIdx;
    programChangeState=1;
    setStompswitchColorRaw(0);
    //genericStompSwitchCallback(0,data);
}

static void stompSwitch2Pressed()
{

    longPressCnt = getTickValue();
}

static void stompswitch2Callback(void)
{
    if (longPressCnt != 0) // no freeze happened, toggle normally
    {
        uint8_t ret = ui.currentProgram->toggleOn();
        if (ret) 
        {
            setStompswitchColorRaw(2 << 2);
        }
        else
        {
            setStompswitchColorRaw(0);
        }
    }
    longPressCnt=0;
    //genericStompSwitchCallback(1,data);
}

static void stompswitch3Callback(void)
{

    ui.currentProgramIdx++;
    if (ui.currentProgramIdx >= N_FX_PROGRAMS )
    {
        ui.currentProgramIdx = N_FX_PROGRAMS-1;
    } 
    programToInitialize=ui.currentProgramIdx;
    programChangeState=1;
    setStompswitchColorRaw(0);
    //genericStompSwitchCallback(2,data);
}


/*
register exit, rotary, knobs and stompswitch callbacks
remove enter callback
register onUpdate, on Create
*/
void enterLevel0()
{
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterPressedCallback);
    registerEnterButtonReleasedCallback(&enterReleasedCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompSwitch2Pressed);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    setStompswitchColorRaw(0);
    ui.defaultOn=0;
    ui.currentProgram->switchOff();
    enterState = 0;
    create();
}
