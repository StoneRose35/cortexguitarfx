extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "drivers/stompswitches.h"
#include "pipicofx/pipicofxui.h"
#include "images/saveOverlay.h"
#include "images/editOverlay.h"
#include "images/routingoverlay.h"
#include "images/ledcoloroverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/toggleswitch_on.h"
#include "images/toggleswitch_off.h"
#include "pipicofx/fxPrograms.h"
#include "images/fwupdateScreen.h"
#include "pcm3060.h"
#include "stringFunctions.h"
#include "drivers/systick.h"
}
#include "pipicofx/MultiAudioProcessor.hpp"


#define LVL11_OVERLAY_NR_SAVE 0
#define LVL11_OVERLAY_NR_EDIT_ROUTING 1
#define LVL11_OVERLAY_NR_SYSTEMSETTINGS 2
#define LVL11_OVERLAY_NR_ABOUT 3
#define LVL11_OVERLAY_NR_FWUPDATE 4

#define OM_NONE 0
#define OM_OVERLAYS 1
#define OM_LED_COLOR 2
#define OM_ABOUT 8
#define OM_FIRMWARE_UPDATE 9
#define OM_SAVE 10


extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
extern volatile uint16_t initialKnobValues[3];
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];

static volatile uint32_t longPressCntA=0;
static volatile uint32_t longPressCntB=0;
static volatile uint32_t longPressCntC=0;
static uint8_t channelState=0xf; // bits 0-1 HiZ, bits2-3 mic, 0: off  1: on, 3: unknown
static void drawPreset(BwImageStruct*imgBuffer);
static void drawPrograms(BwImageType* imgBuffer);
static uint8_t currentVolume;
static uint8_t overlayMode=0;
static uint8_t overlayNr=0xFF;
static const BwImageTypeConst* overlays[]={
    &saveOverlay_streamimg,
    &routingoverlay_streamimg,
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};

static void create()
{
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;
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
    drawPreset(imgBuffer);
    drawPrograms(imgBuffer);
    drawBottomPanel(&bottomPane);
    drawImage(0,48,(BwImageTypeConst*)&bottomPane,imgBuffer);



    if (overlayMode == OM_OVERLAYS)
    {
        drawImage(41,0,overlays[overlayNr],imgBuffer);
    }
    else if (overlayMode == OM_ABOUT)
    {
        drawAbout(imgBuffer);
    }
    else if (overlayMode == OM_FIRMWARE_UPDATE)
    {
        drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
    }
    else if (overlayMode == OM_SAVE)
    {
        clearSquareInt(64-35,4,64+35,22,imgBuffer);
        drawRectFrame(64-35,4,64+35,22,imgBuffer);
        drawText(64-35+2,4+2+8,"Enter:Save",imgBuffer,0);
        drawText(64-35+2,4+2+16,"Exit:Revert",imgBuffer,0);
    }


    if (initialKnobValues[0]==0xFFFF)
    {
        if ((channelState & 0x3)==0)
        {
            drawImage(59,4,&toggleswitch_off_streamimg,imgBuffer);
        }
        else
        {
            drawImage(59,4,&toggleswitch_on_streamimg,imgBuffer);
        }
        drawText(64-9,46,"Mic",imgBuffer,0);
    }

    if (initialKnobValues[1]==0xFFFF)
    {
        if (((channelState>>2) & 0x3)==0)
        {
            drawImage(59,4,&toggleswitch_off_streamimg,imgBuffer);
        }
        else
        {
            drawImage(59,4,&toggleswitch_on_streamimg,imgBuffer);
        }
        drawText(64-9,46,"HiZ",imgBuffer,0);
    }

    if (initialKnobValues[2]==0xFFFF) // master volume is being edited, draw current position
    {
        drawMasterVolume(currentVolume,imgBuffer);
    }
    

    if (longPressCntA > 0 && getTickValue() - longPressCntA > LONGPRESS_DURATION_SYSTICKS 
        && ((FxProgram*)audioProcessor.getFxProgram(0)) != nullptr
        && ((FxProgram*)audioProcessor.getFxProgram(0))->isFreezable() 
        && ((FxProgram*)audioProcessor.getFxProgram(0))->isOn())
    {
        ((FxProgram*)audioProcessor.getFxProgram(0))->freeze();
        longPressCntA=0;
        setStompswitchColor(0,1);
    }
    if (longPressCntB > 0 && getTickValue() - longPressCntB > LONGPRESS_DURATION_SYSTICKS 
        && ((FxProgram*)audioProcessor.getFxProgram(1)) != nullptr
        && ((FxProgram*)audioProcessor.getFxProgram(1))->isFreezable() 
        && ((FxProgram*)audioProcessor.getFxProgram(1))->isOn())
    {
        ((FxProgram*)audioProcessor.getFxProgram(1))->freeze();
        longPressCntB=0;
        setStompswitchColor(1,1);
    }
    if (longPressCntC > 0 && getTickValue() - longPressCntC > LONGPRESS_DURATION_SYSTICKS 
        && ((FxProgram*)audioProcessor.getFxProgram(2)) != nullptr
        && ((FxProgram*)audioProcessor.getFxProgram(2))->isFreezable() 
        && ((FxProgram*)audioProcessor.getFxProgram(2))->isOn())
    {
        ((FxProgram*)audioProcessor.getFxProgram(2))->freeze();
        longPressCntC=0;
        setStompswitchColor(2,1);
    }
}


static void knob0Callback(uint16_t val)
{
    if (ui.enterState == 1)
    {
        if ((val > initialKnobValues[0] && (val - initialKnobValues[0]) >KNOB_HYSTERESIS) || (val < initialKnobValues[0] && (initialKnobValues[0]-val) >KNOB_HYSTERESIS))
        {
            initialKnobValues[0] = 0xFFFF;
            ui.bypassEnterReleased = 1;
            initialKnobValues[1]=getChannel1Value();
            initialKnobValues[2]=getChannel2Value();
            if (val > 2047 && (channelState & 0x3)==0)
            {
                pcm3060SetInputState(PCM3060_CHANNEL_LEFT,1);
                channelState &= ~0x3;
                channelState |= 0x1;
            }
            else if (val <= 2047 && (channelState & 0x3)!=0 )
            {
                pcm3060SetInputState(PCM3060_CHANNEL_LEFT,0);
                channelState &= ~0x3;
            }
        }
    }
}

static void knob1Callback(uint16_t val)
{
    if (ui.enterState == 1)
    {
        if ((val > initialKnobValues[1] && (val - initialKnobValues[1]) >KNOB_HYSTERESIS) || (val < initialKnobValues[1] && (initialKnobValues[1]-val) >KNOB_HYSTERESIS))
        {
            initialKnobValues[1] = 0xFFFF;
            ui.bypassEnterReleased = 1;
            initialKnobValues[0]=getChannel0Value();
            initialKnobValues[2]=getChannel2Value();
            if (val > 2047 && ((channelState >> 2) & 0x3)==0 )
            {
                pcm3060SetInputState(PCM3060_CHANNEL_RIGHT,1);
                channelState &= ~(0x3 << 2);
                channelState |= (0x1 << 2);
            }
            else if (val <= 2047 && (((channelState>>2) & 0x3)!=0))
            {
                pcm3060SetInputState(PCM3060_CHANNEL_RIGHT,0);
                channelState &= ~(0x3 << 2);
            }
        }
    }
}

static void knob2Callback(uint16_t val)
{
    if (ui.enterState == 1)
    {
        if ((val > initialKnobValues[2] && (val - initialKnobValues[2]) >KNOB_HYSTERESIS) || (val < initialKnobValues[2] && (initialKnobValues[2]-val) >KNOB_HYSTERESIS))
        {
            initialKnobValues[2]= 0xFFFF;
            ui.bypassEnterReleased = 1;
            currentVolume = (uint8_t)(val >> 4);
            pcm3060SetOutputVolume(PCM3060_CHANNEL_BOTH,currentVolume);
            initialKnobValues[0]=getChannel0Value();
            initialKnobValues[1]=getChannel1Value();
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
            overlayNr = LVL11_OVERLAY_NR_SAVE;
            uiStackPush(0xFF);
            break;
        case OM_OVERLAYS:
            if (overlayNr == LVL11_OVERLAY_NR_SAVE)
            {
                overlayMode = OM_SAVE;
            }
            else if (overlayNr == LVL11_OVERLAY_NR_EDIT_ROUTING)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(10);
                enterLevel10();
            }
            else if (overlayNr == LVL11_OVERLAY_NR_SYSTEMSETTINGS)
            {
                overlayMode = OM_NONE;
                uiStackPop();
                uiStackPush(10);   
                enterLevel5();
            }/*
            else  if (overlayNr == LVL11_OVERLAY_NR_LEDCOLOR)
            {
                overlayMode = OM_LED_COLOR;
            } */
            else if (overlayNr == LVL11_OVERLAY_NR_ABOUT)
            {
                overlayMode = OM_ABOUT;
            }
            else if (overlayNr == LVL11_OVERLAY_NR_FWUPDATE)
            {
                overlayMode = OM_FIRMWARE_UPDATE;
            }
            break;
        case OM_SAVE:
            overlayMode = OM_NONE;
            parametersToPreset(presets + currentPreset,&audioProcessor);
            savePreset(presets+currentPreset,currentBank*3 + currentPreset);
            break;    
    }
}

static void exitCallback()
{
    // remove overlay menu (if there)
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
            //drawImage(41,0,overlays[overlayNr],imgBuffer);
            break;
        case OM_NONE:
            if (encoderDelta > 0 && ui.currentProgramPosition < 2)
            {
                ui.currentProgramPosition++;
                ui.currentParameterIdx=0;
                if (audioProcessor.getFxProgram(ui.currentProgramPosition) != nullptr)
                {
                    ui.currentProgram = (FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition);
                    ui.currentParameter = ui.currentProgram->getParameter(0);
                }
                else
                {
                    ui.currentProgram = 0;
                    ui.currentParameter = 0;
                }
            }
            else if (encoderDelta < 0 && ui.currentProgramPosition > 0)
            {
                ui.currentProgramPosition--;
                ui.currentParameterIdx=0;
                if (audioProcessor.getFxProgram(ui.currentProgramPosition) != nullptr)
                {
                    ui.currentProgram = (FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition);
                    ui.currentParameter = ui.currentProgram->getParameter(0);
                }
                else
                {
                    ui.currentProgram = 0;
                    ui.currentParameter = 0;
                }
            }
            break;
        /*
        case OM_LED_COLOR:
            if (ui.currentProgramPosition == 0)
            {
                if (encoderDelta > 0 && (presets + currentPreset)->ledColorA < 3)
                {   
                    (presets + currentPreset)->ledColorA++;
                }
                else if (encoderDelta < 0 && (presets + currentPreset)->ledColorA > 1)
                {
                    (presets + currentPreset)->ledColorA--;
                } 
            } else if (ui.currentProgramPosition == 1)
            {
                if (encoderDelta > 0 && (presets + currentPreset)->ledColorB < 3)
                {   
                    (presets + currentPreset)->ledColorB++;
                }
                else if (encoderDelta < 0 && (presets + currentPreset)->ledColorB > 1)
                {
                    (presets + currentPreset)->ledColorB--;
                } 
            }
            else if (ui.currentProgramPosition == 2)
            {
                if (encoderDelta > 0 && (presets + currentPreset)->ledColorC < 3)
                {   
                    (presets + currentPreset)->ledColorC++;
                }
                else if (encoderDelta < 0 && (presets + currentPreset)->ledColorC > 1)
                {
                    (presets + currentPreset)->ledColorC--;
                } 
            }
        */
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

static void stompSwitch1Pressed()
{
    if (ui.enterState == 1)
    {
        ui.currentProgram = (FxProgram*)audioProcessor.getFxProgram(0);
        ui.currentProgramPosition = 0;
        ui.currentParameterIdx = 0;
        ui.bypassEnterReleased = 1;
        ui.mode = PPFX_MODE_STOMPBOX;
        //uiStackPush(11);
        uiSwitchMode();
        return;
    }
    longPressCntA = getTickValue();
}


static void stompSwitch2Pressed()
{
    if (ui.enterState == 1)
    {
        ui.currentProgram = (FxProgram*)audioProcessor.getFxProgram(1);
        ui.currentProgramPosition = 1;
        ui.currentParameterIdx = 0;
        ui.bypassEnterReleased = 1;
        ui.mode = PPFX_MODE_STOMPBOX;
        //uiStackPush(11);
        uiSwitchMode();
        return;
    }
    longPressCntB = getTickValue();
}


static void stompSwitch3Pressed()
{
    if (ui.enterState == 1)
    {
        ui.currentProgram = (FxProgram*)audioProcessor.getFxProgram(2);
        ui.currentProgramPosition = 2;
        ui.currentParameterIdx = 0;
        ui.bypassEnterReleased = 1;
        ui.mode = PPFX_MODE_STOMPBOX;
        //uiStackPush(11);
        uiSwitchMode();
        return;
    }
    longPressCntC = getTickValue();
}

static void stompswitch1Callback()
{
    if (longPressCntA != 0 && audioProcessor.getFxProgram(0) != nullptr) // no freeze happened, toggle normally
    {
        uint8_t ret = ((FxProgram*)audioProcessor.getFxProgram(0))->toggleOn();
        if (ret) 
        {
            setStompswitchColor(0,2);
        }
        else
        {
            setStompswitchColor(0,0);
        }
    }
    longPressCntA = 0;
}

static void stompswitch2Callback()
{
    if (longPressCntB != 0 && audioProcessor.getFxProgram(1) != nullptr) // no freeze happened, toggle normally
    {
        uint8_t ret = ((FxProgram*)audioProcessor.getFxProgram(1))->toggleOn();
        if (ret) 
        {
            setStompswitchColor(1,2);
        }
        else
        {
            setStompswitchColor(1,0);
        }
    }
    longPressCntB = 0;
}

static void stompswitch3Callback()
{
    if (longPressCntC != 0 && audioProcessor.getFxProgram(2) != nullptr) // no freeze happened, toggle normally
    {
        uint8_t ret = ((FxProgram*)audioProcessor.getFxProgram(2))->toggleOn();
        if (ret) 
        {
            setStompswitchColor(2,2);
        }
        else
        {
            setStompswitchColor(2,0);
        }
    }
    longPressCntC = 0;
}

void enterLevel11()
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
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerStompswitch1PressedCallback(&stompSwitch1Pressed);
    registerStompswitch2PressedCallback(&stompSwitch2Pressed);
    registerStompswitch3PressedCallback(&stompSwitch3Pressed);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    for (uint8_t c=0;c<3;c++)
    {
        if (audioProcessor.getFxProgram(c)!= nullptr)
        {
            if (((FxProgram*)audioProcessor.getFxProgram(c))->isOn())
            {
                setStompswitchColor(c,2);
            }
            else if (((FxProgram*)audioProcessor.getFxProgram(c))->isFrozen())
            {
                setStompswitchColor(c,1);
            }
            else
            {
                setStompswitchColor(c,0);
            }
        }
        else
        {
            setStompswitchColor(c,0);
        }
    }
    ui.mode = PPFX_MODE_PEDALBOARD;
    ui.enterState = 0;
    create();
}

static void drawPreset(BwImageStruct*imgBuffer)
{
    char strbfr[24];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    *(strbfr) = 0;
    appendToString(strbfr,presets[currentPreset].name);
    drawText(0,18,strbfr,imgBuffer,font);
    
}


static void drawPrograms(BwImageType* imgBuffer)
{
    char lineBuffer[24];
    const char * ABC[3]={"A:","B:","C:"}; 
    for (uint8_t c=0;c<audioProcessor.getProgramListLength();c++)
    {
        if (audioProcessor.getFxProgram(c) != nullptr)
        {
            lineBuffer[0]=0;
            appendToString(lineBuffer,*(ABC + c));
            appendToString(lineBuffer,((FxProgram*)audioProcessor.getFxProgram(c))->getName());
            drawText(0,28 + c*8,lineBuffer,imgBuffer,0);
        }               
    }
}
