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
#include "images/routingoverlay.h"
#include "images/saveOverlay.h"
#include "images/freezable.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "systick.h"
#include "bootloader_activation.h"
#include "pcm3060.h"
#include "gen/version.h"
#include "pipicofx/pipicofxui.h"
}
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/MultiAudioProcessor.hpp"
#include "pipicofx/picofxCore.hpp"


/** 
 Mode "Program Change" / 20 Stompboxes in one
 the two outermost footswitches change between programs (left:down, right up), the middle footswitch toggles bypass/on for the 
 program/effect chosen. The Three Knob act in an analog manner always reading out the current values
 Rotary: also changes between effects
 Rotary+Enter: Change Mode
 Enter: shows overlay allowing to jump to various submodes
 Exit: lock/unlock knobs
 Knobs: edit parameters assigned to knobs
*/
static void knob0Callback(uint16_t val);
static void knob1Callback(uint16_t val);
static void knob2Callback(uint16_t val);
static void drawParameterDescriptions();
static void drawParameterValues();
static void drawProgramHeader();
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
uint8_t locksymbol[5]={0b01111000,0b01111110,0b01111001,0b01111110,0b01111000 };
BwImageTypeConst lock=
{
    .data = locksymbol,
    .sx=5,
    .sy=8,
    .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES
};
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];
extern const uint8_t stompswitch_progs[];
extern volatile uint16_t initialKnobValues[3];
extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static volatile uint32_t longPressCnt=0;
static volatile uint32_t lastTick;
static uint8_t overlayMode=0;
static uint8_t overlayNr=0xFF;
static uint8_t currentVolume;

static const BwImageTypeConst* overlays[]={
    &saveOverlay_streamimg,
    &editOverlay_streamimg, 
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};


#define LVL0_OVERLAY_NR_SAVE 0
#define LVL0_OVERLAY_NR_EDIT 1
#define LVL0_OVERLAY_NR_SYSTEMSETTINGS 2
#define LVL0_OVERLAY_NR_ABOUT 3
#define LVL0_OVERLAY_NR_FWUPDATE 4

#define LVL0_PARAMETER_DISPLAY_DURATION 132

#define OM_NONE 0
#define OM_OVERLAYS 1
#define OM_SAVE_REVERT 2
#define OM_ABOUT 3
#define OM_FIRMWARE_UPDATE 4

static void create()
{
    lock.data =locksymbol;
    lastTick=getTickValue();
    if (ui.currentProgram == nullptr)
    {
        return;
    }
    /*
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
    }*/
    initialKnobValues[0]=getChannel0Value();
    initialKnobValues[1]=getChannel1Value();
    initialKnobValues[2]=getChannel2Value();
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;
    uint32_t t=getTickValue();
    BwImageType* imgBuffer = getImageBuffer();
    uint8_t bottomPaneData[256];
    BwImageType bottomPane = {
        .data = bottomPaneData,
        .sx=128,
        .sy=16,
        .type = BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize=256
    };
    clearImage(imgBuffer);
    drawProgramHeader();
    drawBottomPanel(&bottomPane);
    drawImage(0,48,(BwImageTypeConst*)&bottomPane,imgBuffer);

    if (overlayMode != OM_OVERLAYS)
    {
        if ((t - lastTick) < LVL0_PARAMETER_DISPLAY_DURATION)
        {
            drawParameterDescriptions();
        }
        else
        {
            drawParameterValues();
        }
    }
    if (t < lastTick || t > lastTick + (LVL0_PARAMETER_DISPLAY_DURATION << 1))
    {
        lastTick = t;
    }
    if (longPressCnt > 0 && getTickValue() - longPressCnt > LONGPRESS_DURATION_SYSTICKS && ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->isFreezable() && ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->isOn())
    {
        ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->freeze();
        longPressCnt=0;
        setStompswitchColorRaw(1 << 2);
    }

    if (overlayMode == OM_OVERLAYS)
    {
        drawImage(41,0,overlays[overlayNr],imgBuffer);
    }
    else if (overlayMode == OM_SAVE_REVERT)
    {
        clearSquareInt(64-35,4,64+35,22,imgBuffer);
        drawRectFrame(64-35,4,64+35,22,imgBuffer);
        drawText(64-35+2,4+1+8,"Enter:Save",imgBuffer,0);
        drawText(64-35+2,4+1+16,"Exit:Revert",imgBuffer,0);
    }
    //1[programsToInitialize]=0xFF;
    //2[programsToInitialize]=0xFF;
    //((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->switchOff();
    //ui.enterState = 0;
    else if (overlayMode == OM_ABOUT)
    {
        drawAbout(imgBuffer);
    }
    else if (overlayMode == OM_FIRMWARE_UPDATE)
    {
        drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
    }

    if (initialKnobValues[2]==0xFFFF) // master volume is being edited, draw current position
    {
        drawMasterVolume(currentVolume,imgBuffer);
    }
    


}

static inline void knobCallback(uint16_t val,uint8_t control)
{
    if (ui.locked == 0 && audioProcessor.getFxProgram(ui.currentProgramPosition) != nullptr)
    {
        for (uint8_t c=0;c<((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount();c++)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter(c)->getControl()==control)
            {
                ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter(c)->parameterCallback(val);
                programsToInitialize[ui.currentProgramPosition] = ui.currentProgramIdx; // tag
            }
        }  
    } 
}



static void knob0Callback(uint16_t val)
{
    BwImageType* imgBuffer = getImageBuffer();
    if (ui.enterState == 0)
    {
        knobCallback(val,0);
    }
    else 
    {
        if ((val > initialKnobValues[0] && (val - initialKnobValues[0]) >KNOB_HYSTERESIS) || (val < initialKnobValues[0] && (initialKnobValues[0]-val) >KNOB_HYSTERESIS))
        {
            initialKnobValues[0]=0xFFFF;
            ui.bypassEnterReleased = 1;
            if (val > 2047 && ui.locked == 0)
            {
                ui.locked = 1;
                drawImage(122,0,&lock,imgBuffer);
            }
            else if ( val< 2047 && ui.locked == 1)
            {
                ui.locked = 0;
                clearSquareInt(122,0,128,8,imgBuffer);
            }
        }
    }
}

static void knob1Callback(uint16_t val)
{
    knobCallback(val,1);
}

static void knob2Callback(uint16_t val)
{
    if (ui.enterState == 0)
    {
        knobCallback(val,2);
    }
    else
    {
        if ((val > initialKnobValues[2] && (val - initialKnobValues[2]) >KNOB_HYSTERESIS) || (val < initialKnobValues[2] && (initialKnobValues[2]-val) >KNOB_HYSTERESIS))
        {
            initialKnobValues[2]= 0xFFFF;
            ui.bypassEnterReleased = 1;
            currentVolume = (uint8_t)(val >> 4);
            pcm3060SetOutputVolume(PCM3060_CHANNEL_BOTH,currentVolume);
        }
    }
}

static void enterPressedCallback()
{
    ui.enterState = 1;
    initialKnobValues[0]=getChannel0Value();
    initialKnobValues[1]=getChannel1Value();
    initialKnobValues[2]=getChannel2Value();
}

static void enterReleasedCallback(void) 
{
    ui.enterState=0;
    if (ui.bypassEnterReleased == 1)
    {
        initialKnobValues[0]=getChannel0Value();
        initialKnobValues[1]=getChannel1Value();
        initialKnobValues[2]=getChannel2Value();
        ui.bypassEnterReleased = 0;
        return;
    }

    switch (overlayMode)
    {
        case OM_NONE:
            overlayMode = OM_OVERLAYS;
            overlayNr = LVL0_OVERLAY_NR_SAVE;
            uiStackPush(0xFF);
            break;
        case OM_OVERLAYS:
            if (overlayNr == LVL0_OVERLAY_NR_SAVE)
            {
                overlayMode = OM_SAVE_REVERT;
            }
            else if (overlayNr == LVL0_OVERLAY_NR_EDIT)
            {
                //overlayMode = OM_NONE;
                //uiStackPop();
                uiStackPush(0);   
                enterLevel2();
            }
            else if (overlayNr == LVL0_OVERLAY_NR_SYSTEMSETTINGS)
            {
                //overlayMode = OM_NONE;
                //uiStackPop();
                uiStackPush(0);   
                enterLevel5();
            }
            else if (overlayNr == LVL0_OVERLAY_NR_ABOUT)
            {
                overlayMode = OM_ABOUT;
            }
            else if (overlayNr == LVL0_OVERLAY_NR_FWUPDATE)
            {
                overlayMode = OM_FIRMWARE_UPDATE;
            }
            break;
        case OM_SAVE_REVERT:
            overlayMode = OM_NONE;
            parametersToPreset(presets + currentPreset,&audioProcessor);
            savePreset(presets+currentPreset,currentBank*3 + currentPreset);
            //uiStackPop();
            //uiStackPush(0);
            break;
    }

}

static void exitCallback()
{
    BwImageType* imgBuffer = getImageBuffer();
    // remove overlay menu (if there)
    switch (overlayMode)
    {
        case OM_NONE:
            //
            //if (uiStackCurrent() == 0x0)
            //{
            ui.locked ^=1;
            if (ui.locked != 0)
            {
                drawImage(122,0,&lock,imgBuffer);
            }
            else
            {
                clearSquareInt(122,0,128,8,imgBuffer);
            }

            break;
        case OM_OVERLAYS:
            overlayNr=0xFF;
            overlayMode = OM_NONE;
            break;
        case OM_SAVE_REVERT:
            overlayNr=0xFF;
            overlayMode = OM_NONE;
            if (loadPreset(presets+currentPreset,currentBank*3 + currentPreset)!=0)
            {
                generateEmptyPreset(presets+currentPreset,currentBank,currentPreset);
            }
            if (programsToInitialize[0] != presets[currentPreset].programNrA || 
                programsToInitialize[1] != presets[currentPreset].programNrB ||
                programsToInitialize[2] != presets[currentPreset].programNrC)
            {
                programChangeState = 1;
            }
            else
            {
                applyPreset(presets+currentPreset,&audioProcessor);
            }
            break;
        case OM_ABOUT:
        case OM_FIRMWARE_UPDATE:
            overlayMode = OM_OVERLAYS;
            break;

    }
}

static void rotaryCallback(int16_t encoderDelta)
{
    if (ui.enterState==1)
    {
        ui.bypassEnterReleased = 1;
        if (encoderDelta > 0)
        {
            ui.mode++;
            if (ui.mode > 4)
            {
                ui.mode=4;
            }
        }
        else
        {
            ui.mode--;
            if (ui.mode > 4)
            {
                ui.mode=0;
            }
        }
        uiSwitchMode();
        return;
    }
    switch (overlayMode)
    {
        case OM_OVERLAYS:
            if (encoderDelta > 0)
            {
                overlayNr++;
                if (overlayNr > (sizeof(overlays)/(sizeof(BwImageTypeConst*)))-1)
                {
                    overlayNr=(sizeof(overlays)/(sizeof(BwImageTypeConst*)))-1;
                }
            }
            else
            {
                overlayNr--;
                if (overlayNr > (sizeof(overlays)/(sizeof(BwImageTypeConst*)))-1)
                {
                    overlayNr=0;
                }

            }
            break;
        case OM_NONE:
            if (programChangeState==0)
            {
                programsToInitialize[0]=0x7F;
                programsToInitialize[1]=0x7F;
                programsToInitialize[2]=0x7F;
                if (encoderDelta > 0)
                {
                    encoderDelta = 1;
                }
                else
                {
                    encoderDelta = -1;
                }
                if (ui.currentProgramIdx >= N_FX_PROGRAMS-1 && encoderDelta > 0)
                {
                    ui.currentProgramIdx = 0xFF;
                } 
                else if (ui.currentProgramIdx > N_FX_PROGRAMS-1 && encoderDelta < 0)
                {
                    ui.currentProgramIdx = N_FX_PROGRAMS-1;
                }
                else if (ui.currentProgramIdx == 0 && encoderDelta < 0 )
                {
                    ui.currentProgramIdx = 0;
                }
                else if (ui.currentProgramIdx == 0xFF && encoderDelta > 0)
                {
                    ui.currentProgramIdx = 0xFF;
                }
                else
                {
                    ui.currentProgramIdx += encoderDelta;
                }
                programsToInitialize[ui.currentProgramPosition]=ui.currentProgramIdx;
                programChangeState=1;
                setStompswitchColorRaw(0);

            }
            break;
    }
}


static void stompswitch1Callback(void)
{
    if (ui.enterState == 0)
    {
        ui.currentProgramIdx--;
        if (ui.currentProgramIdx >= N_FX_PROGRAMS)
        {
            ui.currentProgramIdx = 0;
        }
        programsToInitialize[ui.currentProgramPosition]=ui.currentProgramIdx;
        programChangeState=1;
        setStompswitchColorRaw(0);
    }
    else
    {
        ui.bypassEnterReleased = 1;
        if (ui.currentParameterIdx >0)
        {
            ui.currentParameterIdx--;
            ui.currentParameter = ui.currentProgram->getParameter(ui.currentParameterIdx);
        }
    }
}

static void leftCallback(void)
{
    ui.mode--;
    if (ui.mode > 4)
    {
        ui.mode=0;
    }
    uiSwitchMode();
}

static void rightCallback(void)
{
    ui.mode++;
    if (ui.mode > 4)
    {
        ui.mode=4;
    }
    uiSwitchMode(); 
}

static void stompSwitch2Pressed()
{

    longPressCnt = getTickValue();
}

static void stompswitch2Callback(void)
{
    if (longPressCnt != 0 && audioProcessor.getFxProgram(ui.currentProgramPosition) != nullptr) // no freeze happened, toggle normally
    {
        uint8_t ret = ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->toggleOn();
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
}

static void stompswitch3Callback(void)
{
    if (ui.enterState==0)
    {
        ui.currentProgramIdx++;
        if (ui.currentProgramIdx >= N_FX_PROGRAMS )
        {
            ui.currentProgramIdx = N_FX_PROGRAMS-1;
        } 
        programsToInitialize[ui.currentProgramPosition]=ui.currentProgramIdx;
        programChangeState=1;
        setStompswitchColorRaw(0);
    }
    else
    {
        ui.bypassEnterReleased = 1;
        if (ui.currentParameterIdx <  ui.currentProgram->getParameterCount()-1)
        {
            ui.currentParameterIdx++;
            ui.currentParameter = ui.currentProgram->getParameter(ui.currentParameterIdx);
        }
    }
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
    registerLeftButtonPressedCallback(&leftCallback);
    registerRightButtonPressedCallback(&rightCallback);
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
    if (ui.currentProgram != nullptr && ui.currentProgram->isFrozen())
    {
        setStompswitchColorRaw(1 << 2);
    }
    else if (ui.currentProgram!=nullptr && ui.currentProgram->isOn())
    {
        setStompswitchColorRaw(2 << 2);
    }
    else
    {
        setStompswitchColorRaw(0);
    }
    ui.defaultOn=0;
    ui.mode = PPFX_MODE_STOMPBOX;
    ui.locked = 1;
    uiStackPush(0xFF);
    create();
}


static void drawParameterDescriptions()
{
    char lineBuffer[24];
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt(0,20,110,20+3*8,imgBuffer);
    if (ui.currentProgram == nullptr)
    {
        return;
    }
    for (uint8_t c=0;c<ui.currentProgram->getParameterCount();c++)
    {
        if (ui.currentProgram->getParameter(c)->getControl() == 0)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P1:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,20 + 1*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 1)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P2:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,20 + 2*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 2)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P3:");
            appendToString(lineBuffer,ui.currentProgram->getParameter(c)->getParameterName());
            drawText(0,20 + 3*8,lineBuffer,imgBuffer,0);
        }                
    }
}

static void drawParameterValues()
{
    char lineBuffer[24];
    
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt(0,20,110,20+3*8,imgBuffer);
    if (ui.currentProgram == nullptr)
    {
        return;
    }
    for (uint8_t c=0;c<ui.currentProgram->getParameterCount();c++)
    {
        if (ui.currentProgram->getParameter(c)->getControl() == 0)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P1:");
            ui.currentProgram->getParameter(c)->parameterDisplay(lineBuffer+3);
            drawText(0,20 + 1*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 1)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P2:");
            ui.currentProgram->getParameter(c)->parameterDisplay(lineBuffer+3);
            drawText(0,20 + 2*8,lineBuffer,imgBuffer,0);
        }
        if (ui.currentProgram->getParameter(c)->getControl() == 2)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,"P3:");
            ui.currentProgram->getParameter(c)->parameterDisplay(lineBuffer+3);
            drawText(0,20 + 3*8,lineBuffer,imgBuffer,0);
        }                
    }
}

static void drawProgramHeader()
{
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt(0,0,128,32,imgBuffer);
    if (ui.currentProgram == nullptr)
    {
        return;
    }
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
}

