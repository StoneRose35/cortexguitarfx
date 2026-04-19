extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "drivers/stompswitches.h"
#include "pipicofx/pipicofxui.h"
#include "images/pipicofx_param_2_scaled.h"
#include "images/pipicofx_param_1_scaled.h"
#include "images/saveOverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/fwupdateScreen.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "stringFunctions.h"
#include "systick.h"
}
#include "pipicofx/MultiAudioProcessor.hpp"


extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
extern volatile uint16_t initialKnobValues[3];
static volatile uint8_t bmKnobsLocked; //bitmap, 0: unlocked, 1: locked, lsb is knob 1, lsb+1 is knob 1 and lsb+2 is knob 2
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];
static uint8_t currentParameterPage = 1;
static uint8_t totalParameterPages;
static uint8_t overlayNr=0xFF;
static uint8_t overlayMode=0;
static volatile uint32_t longPressCnt=0;

#define LVL2_OVERLAY_NR_SAVE 0
#define LVL2_OVERLAY_NR_SYSTEMSETTINGS 1
#define LVL2_OVERLAY_NR_ABOUT 2
#define LVL2_OVERLAY_NR_FWUPDATE 3

#define OM_NONE 0
#define OM_OVERLAYS 1
#define OM_ABOUT 8
#define OM_FIRMWARE_UPDATE 9
#define OM_SAVE 10

static const BwImageTypeConst* overlays[]={
    &saveOverlay_streamimg, 
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};

static void create()
{
    initialKnobValues[0]=getChannel0Value();
    initialKnobValues[1]=getChannel1Value();
    initialKnobValues[2]=getChannel2Value();
    bmKnobsLocked = 0x7;
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;
    BwImageType * img = getImageBuffer();
    char lineBfr[32];
    int16_t potVal;
    uint8_t bottomPaneData[256];
    BwImageType bottomPane = {
        .data = bottomPaneData,
        .sx=128,
        .sy=16,
        .type = BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize=256
    };
    clearImage(img);
    drawBottomPanel(&bottomPane);
    drawImage(0,48,(BwImageTypeConst*)&bottomPane,img);
    if (ui.currentProgram == nullptr)
    {
        return;
    }
    *lineBfr=0;
    appendToString(lineBfr,ui.currentProgram->getName());
    appendToStringUntil(lineBfr,"               ",20);
    UInt8ToChar(currentParameterPage,lineBfr+17);
    appendToString(lineBfr+17,"/");
    UInt8ToChar(totalParameterPages,lineBfr+19);
    drawText(0,8,lineBfr,img,0);

    if ((currentParameterPage-1)*3 <ui.currentProgram->getParameterCount())
    {
        *lineBfr=0;
        appendToString(lineBfr,ui.currentProgram->getParameter((currentParameterPage-1)*3)->getParameterName());
        uint16_t tail = appendToStringUntil(lineBfr,"          ",14);
        ui.currentProgram->getParameter((currentParameterPage-1)*3)->parameterDisplay(lineBfr + tail);
        drawText(0,16,lineBfr,img,0);
        drawSquareInt(0,17,ui.currentProgram->getParameter((currentParameterPage-1)*3)->rawValue  >> 5,21,img);
        potVal = getChannel0Value() >> 5;
        togglePixel(potVal,17,img);
        togglePixel(potVal,17+1,img);
        togglePixel(potVal,17+2,img);
        togglePixel(potVal,17+3,img);

    }
    if ((currentParameterPage-1)*3 + 1<ui.currentProgram->getParameterCount())
    {
        *lineBfr=0;
        appendToString(lineBfr,ui.currentProgram->getParameter((currentParameterPage-1)*3+1)->getParameterName());
        uint16_t tail = appendToStringUntil(lineBfr,"          ",14);
        ui.currentProgram->getParameter((currentParameterPage-1)*3+1)->parameterDisplay(lineBfr + tail);
        drawText(0,24+5,lineBfr,img,0);
        drawSquareInt(0,24+6,ui.currentProgram->getParameter((currentParameterPage-1)*3+1)->rawValue  >> 5,34,img);
        potVal = getChannel1Value() >> 5;
        togglePixel(potVal,24+6,img);
        togglePixel(potVal,24+6+1,img);
        togglePixel(potVal,24+6+2,img);
        togglePixel(potVal,24+6+3,img);
    }
    if ((currentParameterPage-1)*3 + 2<ui.currentProgram->getParameterCount())
    {
        *lineBfr=0;
        appendToString(lineBfr,ui.currentProgram->getParameter((currentParameterPage-1)*3+2)->getParameterName());
        uint16_t tail = appendToStringUntil(lineBfr,"          ",14);
        ui.currentProgram->getParameter((currentParameterPage-1)*3+2)->parameterDisplay(lineBfr + tail);
        drawText(0,32+10,lineBfr,img,0);
        drawSquareInt(0,43,ui.currentProgram->getParameter((currentParameterPage-1)*3+2)->rawValue  >> 5,47,img);
        potVal = getChannel2Value() >> 5;
        togglePixel(potVal,43,img);
        togglePixel(potVal,43+1,img);
        togglePixel(potVal,43+2,img);
        togglePixel(potVal,43+3,img);
    }

    if (overlayMode == OM_OVERLAYS)
    {
        drawImage(41,0,overlays[overlayNr],img);
    }
    else if (overlayMode == OM_SAVE)
    {
        clearSquareInt(64-35,4,64+35,22,img);
        drawRectFrame(64-35,4,64+35,22,img);
        drawText(64-35+2,4+2+8,"Enter:Save",img,0);
        drawText(64-35+2,4+2+16,"Exit:Revert",img,0);
    }
    else if (overlayMode == OM_ABOUT)
    {
        drawAbout(img);
    }
    else if (overlayMode == OM_FIRMWARE_UPDATE)
    {
        drawImage(0,0,&fwupdateScreen_streamimg,img);
    }


    if (longPressCnt > 0 && getTickValue() - longPressCnt > LONGPRESS_DURATION_SYSTICKS && ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->isFreezable() && ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->isOn())
    {
        ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->freeze();
        longPressCnt=0;
        setStompswitchColorRaw(1 << 2);
    }

    // unlock knobs if knob has been moved over the current parameter value and has moved more than KNOB_HYSTERESIS
    if ((currentParameterPage-1)*3 <ui.currentProgram->getParameterCount() && (bmKnobsLocked & 1)!=0)
    {
        int16_t programParameterVal = ui.currentProgram->getParameter((currentParameterPage-1)*3)->rawValue;
        int16_t knobVal = getChannel0Value();
        
        if (((programParameterVal - knobVal) >= 0 && (programParameterVal - knobVal) < KNOB_HYSTERESIS && ((initialKnobValues[0] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[0] - knobVal <= -KNOB_HYSTERESIS_2)) ||
            ((programParameterVal - knobVal) <= 0 && (knobVal - programParameterVal) < KNOB_HYSTERESIS && ((initialKnobValues[0] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[0] - knobVal <= -KNOB_HYSTERESIS_2)))
        {
            bmKnobsLocked &= ~0x1;
        }
    }
    if ((currentParameterPage-1)*3 + 1 <ui.currentProgram->getParameterCount() && (bmKnobsLocked & (1 << 1))!=0)
    {
        int16_t programParameterVal = ui.currentProgram->getParameter((currentParameterPage-1)*3 + 1)->rawValue;
        int16_t knobVal = getChannel1Value();
        if (((programParameterVal - knobVal) >= 0 && (programParameterVal - knobVal) < KNOB_HYSTERESIS && ((initialKnobValues[1] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[1] - knobVal <= -KNOB_HYSTERESIS_2)) ||
            ((programParameterVal - knobVal) <= 0 && (knobVal - programParameterVal) < KNOB_HYSTERESIS && ((initialKnobValues[1] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[1] - knobVal <= -KNOB_HYSTERESIS_2)))
        {
            bmKnobsLocked &= ~(0x1 << 1);
        }
    }
    if ((currentParameterPage-1)*3 + 2 <ui.currentProgram->getParameterCount() && (bmKnobsLocked & (1 << 2))!=0)
    {
        int16_t programParameterVal = ui.currentProgram->getParameter((currentParameterPage-1)*3 + 2)->rawValue;
        int16_t knobVal = getChannel2Value();
        if (((programParameterVal - knobVal) >= 0 && (programParameterVal - knobVal) < KNOB_HYSTERESIS && ((initialKnobValues[2] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[2] - knobVal <= -KNOB_HYSTERESIS_2)) ||
            ((programParameterVal - knobVal) <= 0 && (knobVal - programParameterVal) < KNOB_HYSTERESIS && ((initialKnobValues[2] - knobVal) >= KNOB_HYSTERESIS_2 || initialKnobValues[2] - knobVal <= -KNOB_HYSTERESIS_2)))
        {
            bmKnobsLocked &= ~(0x1 << 2);
        }
    }
}

static void leftCallback(void)
{
    if (currentParameterPage > 1)
    {
        currentParameterPage--;
        initialKnobValues[0]=getChannel0Value();
        initialKnobValues[1]=getChannel1Value();
        initialKnobValues[2]=getChannel2Value();
        bmKnobsLocked=0x7;
    }
    else
    {
        ui.mode--;
        if (ui.mode > 4)
        {
            ui.mode=0;
        }
        uiSwitchMode();
    }
}

static void rightCallback(void)
{
    if (currentParameterPage < totalParameterPages)
    {
        currentParameterPage++;
        initialKnobValues[0]=getChannel0Value();
        initialKnobValues[1]=getChannel1Value();
        initialKnobValues[2]=getChannel2Value();
        bmKnobsLocked =0x7;
    }
    else
    {
        ui.mode++;
        if (ui.mode > 4)
        {
            ui.mode=4;
        }
        uiSwitchMode(); 
    }
}

static void enterPressedCallback()
{
    ui.enterState = 1;
    initialKnobValues[0]=getChannel0Value();
    initialKnobValues[1]=getChannel1Value();
    initialKnobValues[2]=getChannel2Value();
}

static void enterReleasedCallback()
{
    ui.enterState=0;
    if (ui.bypassEnterReleased == 1)
    {
        ui.bypassEnterReleased = 0;
        return;
    }
        // show overlay menu (if not there)
    switch (overlayMode)
    {
        case OM_NONE:
            overlayMode = OM_OVERLAYS;
            overlayNr = LVL2_OVERLAY_NR_SAVE;
            uiStackPush(0xFF);
            break;
        case OM_OVERLAYS:
            if (overlayNr == LVL2_OVERLAY_NR_SAVE)
            {
                overlayMode = OM_SAVE;
            }
            else if (overlayNr == LVL2_OVERLAY_NR_SYSTEMSETTINGS)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(3);   
                enterLevel5();
            }
            else if (overlayNr == LVL2_OVERLAY_NR_ABOUT)
            {
                overlayMode = OM_ABOUT;
            }
            else if (overlayNr == LVL2_OVERLAY_NR_FWUPDATE)
            {
                overlayMode = OM_FIRMWARE_UPDATE;
            }
            break;
        case OM_SAVE:
            overlayMode = OM_NONE;
            savePreset(presets+currentPreset,currentBank*3 + currentPreset);
            uiStackPop();
            uiStackPush(3);
            break;    
    }
}

static void exitCallback()
{
    switch (overlayMode)
    {
        case OM_OVERLAYS:
            overlayNr=0xFF;
            overlayMode = OM_NONE;
            break;
        case OM_SAVE:
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
            break;
        case OM_ABOUT:
        case OM_FIRMWARE_UPDATE:
            overlayMode = OM_OVERLAYS;
            break;

    }
}


static void stompswitch1Callback(void)
{
    /*
    uint8_t nbStompSwitch;
    nbStompSwitch=getStompSwitchState(1);
    if ((nbStompSwitch & 0x1) == 0x1)
    {
        if (currentParameterPage > 0)
        {
            currentParameterPage--;
        }
    }
    */
}

static void stompSwitch2Pressed()
{

    longPressCnt = getTickValue();
}

static void stompswitch2Callback(void)
{
    /*
    uint8_t nbStompSwitch1, nbStompSwitch3;
    nbStompSwitch1=getStompSwitchState(0);
    nbStompSwitch3=getStompSwitchState(2);
    
    if (((nbStompSwitch1 & 0x1) == 0x1) && ((nbStompSwitch3 & 0x1) == 0x0))
    {
        if (currentParameterPage > 1)
        {
            currentParameterPage--;
        }
    }
    else if (((nbStompSwitch1 & 0x1) == 0x0) && ((nbStompSwitch3 & 0x1) == 0x1))
    {
        if (currentParameterPage < totalParameterPages )
        {
            currentParameterPage++;
        }
    }
    */

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
    /*
    uint8_t nbStompSwitch;
    nbStompSwitch=getStompSwitchState(1);
    if ((nbStompSwitch & 0x1) == 0x1)
    {
        if (currentParameterPage < totalParameterPages)
        {
            currentParameterPage++;
        }
    }
    */
}

static inline void knobCallback(uint16_t val,uint8_t control)
{
    if (ui.enterState == 0 && ui.locked == 0)
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
    /*
    if (ui.enterState == 1)
    {
        if (((val > initialKnobValues[0] && (val - initialKnobValues[0]) >KNOB_HYSTERESIS) || (val < initialKnobValues[0] && (initialKnobValues[0]-val) >KNOB_HYSTERESIS))
            && (currentParameterPage - 1)*3 < ui.currentProgram->getParameterCount())
        {
            initialKnobValues[0] = 0xFFFF;
            ui.bypassEnterReleased = 1;
            initialKnobValues[1]=getChannel1Value();
            initialKnobValues[2]=getChannel2Value();
            ui.currentProgram->getParameter((currentParameterPage - 1)*3)->parameterCallback(val);
            ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3);
        }
    }
    */
    if ((bmKnobsLocked & (1 << 0))==0)
    {
        ui.currentProgram->getParameter((currentParameterPage - 1)*3)->parameterCallback(val);
        ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3);
    }
}

static void knob1Callback(uint16_t val)
{
    /*
    if (ui.enterState == 1)
    {
        if (((val > initialKnobValues[1] && (val - initialKnobValues[1]) >KNOB_HYSTERESIS) || (val < initialKnobValues[1] && (initialKnobValues[1]-val) >KNOB_HYSTERESIS))
        && (currentParameterPage - 1)*3 + 1< ui.currentProgram->getParameterCount())
        {
            initialKnobValues[1] = 0xFFFF;
            ui.bypassEnterReleased = 1;
            initialKnobValues[0]=getChannel0Value();
            initialKnobValues[2]=getChannel2Value();
            ui.currentProgram->getParameter((currentParameterPage - 1)*3+1)->parameterCallback(val);
            ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3+1);
        }
    }*/
    if ((bmKnobsLocked & (1 << 1))==0)
    {
        ui.currentProgram->getParameter((currentParameterPage - 1)*3 + 1)->parameterCallback(val);
        ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3 + 1);
    }
}

static void knob2Callback(uint16_t val)
{
    /*
    if (ui.enterState == 1)
    {
        if (((val > initialKnobValues[2] && (val - initialKnobValues[2]) >KNOB_HYSTERESIS) || (val < initialKnobValues[2] && (initialKnobValues[2]-val) >KNOB_HYSTERESIS))
        && (currentParameterPage - 1)*3 + 2 < ui.currentProgram->getParameterCount())
        {
            initialKnobValues[2] = 0xFFFF;
            ui.bypassEnterReleased = 1;
            initialKnobValues[0]=getChannel0Value();
            initialKnobValues[1]=getChannel1Value();
            ui.currentProgram->getParameter((currentParameterPage - 1)*3+2)->parameterCallback(val);
            ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3+2);
        }
    }*/
    if ((bmKnobsLocked & (1 << 2))==0)
    {
        ui.currentProgram->getParameter((currentParameterPage - 1)*3 + 2)->parameterCallback(val);
        ui.currentParameter = ui.currentProgram->getParameter((currentParameterPage - 1)*3 + 2);
    }
}


static void rotaryCallback(int16_t encoderDelta)
{
    if (ui.enterState == 0)
    {

        if (overlayMode == OM_OVERLAYS)
        {
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
                //drawImage(41,0,overlays[overlayNr],imgBuffer);
        }
        else if (ui.currentParameter != nullptr)
        {
            if (ui.currentParameter->increment > 1) // handle parameter whose increments are
            {                                          // larger than one as discrete param, thus increment only by one
                if (encoderDelta > 1)
                {
                    encoderDelta = 1;
                }
                else if (encoderDelta < -1)
                {
                    encoderDelta = -1;
                }
            }
            ui.currentParameter->rawValue += encoderDelta*ui.currentParameter->increment;
            if (ui.currentParameter->rawValue < 0)
            {
                ui.currentParameter->rawValue = 0;
            }
            else if  (ui.currentParameter->rawValue > ((1 << 12)-1))
            {
                ui.currentParameter->rawValue = ((1 << 12)-1);
            }
            ui.currentParameter->parameterCallback(ui.currentParameter->rawValue);
            initialKnobValues[0]=getChannel0Value();
            initialKnobValues[1]=getChannel1Value();
            initialKnobValues[2]=getChannel2Value();
            bmKnobsLocked = 0x7; // lock all knobs when the encoder has been used to set a parameter

        }
        
    }
    else
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
}

void enterLevel2()
{
    clearCallbackAssignments();
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    registerEnterButtonPressedCallback(&enterPressedCallback);
    registerEnterButtonReleasedCallback(&enterReleasedCallback);
    registerLeftButtonPressedCallback(&leftCallback);
    registerRightButtonPressedCallback(&rightCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompSwitch2Pressed);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    create();
    //currentParameterPage = 1;
    totalParameterPages = ((ui.currentProgram->getParameterCount()+2)/3);
}