extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/fwupdateScreen.h"
#include "images/looperOverlay.h"
#include "images/freezable.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "systick.h"
#include "bootloader_activation.h"
#include "gen/version.h"
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
static uint8_t overlayMode=0;
static uint8_t overlayNr=0xFF;

static const BwImageTypeConst* overlays[]={
    &looperOverlay_streamimg,
    &editOverlay_streamimg, 
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};


#define LVL0_OVERLAY_NR_LOOPER 0
#define LVL0_OVERLAY_NR_EDIT 1
#define LVL0_OVERLAY_NR_SYSTEMSETTINGS 2
#define LVL0_OVERLAY_NR_ABOUT 3
#define LVL0_OVERLAY_NR_FWUPDATE 4
#define LVL0_OVERLAY_NR_ABOUT_SHOWING 5

#define OM_NONE 0
#define OM_OVERLAYS 1

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

    if (ui.currentProgram->isFreezable())
    {
        drawImage(110,16,&freezable_streamimg,imgBuffer);
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
    char strbfr[24];
    BwImageType* imgBuffer = getImageBuffer();
    if (consumeEnterReleased == 1)
    {
        consumeEnterReleased = 0;
        return;
    }

    switch (overlayMode)
    {
        case OM_NONE:
            overlayMode = OM_OVERLAYS;
            overlayNr = LVL0_OVERLAY_NR_LOOPER;
            drawImage(41,0,&looperOverlay_streamimg,imgBuffer);
            uiStackPush(0xFF);
            break;
        case OM_OVERLAYS:
            if (overlayNr == LVL0_OVERLAY_NR_LOOPER)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(0);   
                enterLevel8();
            }
            else if (overlayNr == LVL0_OVERLAY_NR_EDIT)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(0);   
                enterLevel1();
            }
            else if (overlayNr == LVL0_OVERLAY_NR_SYSTEMSETTINGS)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(0);   
                enterLevel5();
            }
            else if (overlayNr == LVL0_OVERLAY_NR_ABOUT)
            {
                overlayMode = OM_NONE;
                overlayNr = LVL0_OVERLAY_NR_ABOUT_SHOWING;
                clearSquareInt(0,0,128,43,imgBuffer);
                *strbfr=0;
                appendToString(strbfr,"About PiPicoFX");
                drawText(0,8,strbfr,imgBuffer,(void*)0);
                drawText(0,16,PI_PICO_FX_VERSION_NR,imgBuffer,(void*)0);
                drawText(0,24,PI_PICO_FX_MCU_BOARD,imgBuffer,(void*)0);
                *strbfr=0;
                appendToString(strbfr,"built ");
                appendToString(strbfr,PI_PICO_FX_BUILD_DATE);
                drawText(0,32,strbfr,imgBuffer,(void*)0);
                *strbfr=0;
                appendToString(strbfr,"      ");
                appendToString(strbfr,PI_PICO_FX_BUILD_TIME);
                drawText(0,40,strbfr,imgBuffer,(void*)0);
            }
            else if (overlayNr == LVL0_OVERLAY_NR_FWUPDATE)
            {
                overlayMode = OM_NONE;
                drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
                DisplayImageStandardAdressing(0,0,128,8,imgBuffer->data);
                jumpToBootloader();
            }
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
    BwImageType* imgBuffer = getImageBuffer();
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
    switch (overlayMode)
    {
        case OM_OVERLAYS:
            if (encoderDelta > 0)
            {
                overlayNr++;
                if (overlayNr > sizeof(overlays)/(sizeof(BwImageTypeConst*)))
                {
                    overlayNr=sizeof(overlays)/(sizeof(BwImageTypeConst*));
                }
            }
            else
            {
                overlayNr--;
                if (overlayNr > sizeof(overlays)/(sizeof(BwImageTypeConst*)))
                {
                    overlayNr=0;
                }

            }
            drawImage(41,0,overlays[overlayNr],imgBuffer);
            break;
        case OM_NONE:
            if (programChangeState==0)
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
                if (programToInitialize == 18) // ugly hack to jump to program 9 when loading the generic distortion based program
                {
                    uiStackPush(0);
                    enterLevel9();
                }
            }
            break;
    }
}


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
