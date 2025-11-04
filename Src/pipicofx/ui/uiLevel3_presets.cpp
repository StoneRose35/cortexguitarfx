extern "C"
{
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/playoverlay.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/fwupdateScreen.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "gen/version.h"
}
#include "pipicofx/FxProgramLoader.hpp"

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static volatile uint8_t overlayNr=0xFF;
static volatile uint8_t bankChanged=0; // flag indicating that the bank has been changed upon stomp switch release
                                       // used to prohibit action when the second stomp switch is released
static const BwImageType* overlays[]={&playoverlay_streamimg,&editOverlay_streamimg, &settingsOverlay_streamimg, &aboutoverlay_streamimg, &fwUpgradeOverlay_streamimg};
extern volatile uint8_t programToInitialize;
extern volatile uint8_t programChangeState;


static uint16_t pot1Val=0;
static uint16_t pot2Val=0;
static uint16_t pot3Val=0;

static void handleBankChange(uint8_t, PiPicoFxUiType*);
static void handlePresetChange(uint8_t , PiPicoFxUiType*);
static void setPresetNr(uint8_t,PiPicoFxUiType*);
static void setPreset(PiPicoFxUiType*);

#define BANK_PRESET_CHANGE_NONE 2
#define BANK_PRESET_CHANGE_INCREASE 1
#define BANK_PRESET_CHANGE_DECREASE 0 

#define OVERLAY_NR_PLAY 0
#define OVERLAY_NR_EDIT 1
#define OVERLAY_NR_SYSTEMSETTINGS 2
#define OVERLAY_NR_ABOUT 3
#define OVERLAY_NR_FWUPDATE 4

static void create(PiPicoFxUiType*data)
{
    char strbfr[24];
    char nrbfr[8];
    uint8_t startY;
    uint16_t val_p1=0xFFFF,val_p2=0xFFFF,val_p3=0xFFFF;
    BwImageType* imgBuffer = getImageBuffer();
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    // display preset Name and Bank Number
    clearImage(imgBuffer);
    *(strbfr) = 0;    
    appendToString(strbfr,"Bank:");
    UInt8ToChar(currentBank,nrbfr);
    appendToString(strbfr,nrbfr);
    drawText(5,21,strbfr,imgBuffer,font);
    *(strbfr) = 0;
    appendToString(strbfr,presets[currentPreset].name);
    drawText(5,42,strbfr,imgBuffer,font);

    for (uint8_t c=0;c<data->currentProgram->getParameterCount();c++)
    {
        uint8_t currentCtrl = data->currentProgram->getParameter(c)->getControl();
        switch (currentCtrl)
        {
        case 0:
            val_p1 = data->currentProgram->getParameter(c)->rawValue;
            break;
        case 1:
            val_p2 = data->currentProgram->getParameter(c)->rawValue;
            break;
        case 2: 
            val_p3 = data->currentProgram->getParameter(c)->rawValue;
        default:
            break;
        }
    }

    if (val_p3 != 0xFFFF)
    {
        startY = ((uint16_t)4096-val_p3)>>6;
        if (startY > 0x3f)
        {
            startY = 0x3f;
        }
        // frame for value of p3
        drawSquareInt(128-4,0,128,64,imgBuffer);
        clearSquareInt(128-4+1,1,128-1,64-1,imgBuffer);
        // value of p1
        drawSquareInt(128-4+1,startY, 128-1,64-1,imgBuffer);
    }

    if (val_p2 != 0xFFFF)
    {
        startY = ((uint16_t)4096-val_p2)>>6;
        if (startY > 0x3f)
        {
            startY = 0x3f;
        }
        // frame for value of p2
        drawSquareInt(128-4-1*6,0,128-1*6,64,imgBuffer);
        clearSquareInt(128-4+1-1*6,1,128-1-1*6,64-1,imgBuffer);
        // value of p2
        drawSquareInt(128-4+1-1*6,startY,128-1-1*6,64-1,imgBuffer);
    }

    if (val_p1 != 0xFFFF)
    {
        startY = ((uint16_t)4096-val_p1)>>6;
        if (startY > 0x3f)
        {
            startY = 0x3f;
        }
        // frame for value of p1
        drawSquareInt(128-4-2*6,0,128-2*6,64,imgBuffer);
        clearSquareInt(128-4+1-2*6,1,128-1-2*6,64-1,imgBuffer);
        // value of p3
        drawSquareInt(128-4+1-2*6,startY,128-1-2*6,64-1,imgBuffer);
    }
    
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{    
    uint16_t yval; 
    BwImageType* imgBuffer = getImageBuffer();
    // draw Level bars
    clearSquareInt(0,56,128-3*6,64,imgBuffer);
    //in
    drawSquareInt(0,58,0 + ((avgInput)*(128-3*6))/128,60,imgBuffer);
    //out
    drawSquareInt(0,62,0 + ((avgOutput)*(128-3*6))/128,64,imgBuffer);

    // draw current position of potentiometers
    clearSquareInt(122,0,124,64,imgBuffer);
    yval =  (4096-pot3Val)>>6;
    if (yval == 0)
    {
        yval = 1;
    }
    else if (yval == 63)
    {
        yval = 62;
    }
    drawSquareInt(122,yval-1,124,yval+1,imgBuffer);

    clearSquareInt(122-1*6,0,124-1*6,64,imgBuffer);
    yval =  (4096-pot2Val)>>6;
    if (yval == 0)
    {
        yval = 1;
    }
    else if (yval == 63)
    {
        yval = 62;
    }
    drawSquareInt(122-1*6,yval-1,124-1*6,yval+1,imgBuffer);

    clearSquareInt(122-2*6,0,124-2*6,64,imgBuffer);
    yval =  (4096-pot1Val)>>6;
    if (yval == 0)
    {
        yval = 1;
    }
    else if (yval == 63)
    {
        yval = 62;
    }
    drawSquareInt(122-2*6,yval-1,124-2*6,yval+1,imgBuffer);

    DisplayWriteFramebufferAsync(imgBuffer->data);
}

static void enterCallback(PiPicoFxUiType*data) 
{
    BwImageType* imgBuffer = getImageBuffer();
    char strbfr[24];
    // show overlay menu (if not there)
    if (overlayNr == 0xFF)
    {
        overlayNr = OVERLAY_NR_PLAY;
        drawImage(41,0,&playoverlay_streamimg,imgBuffer);
        uiStackPush(data,0xFF);
    }
    else
    {
        uiStackPop(data);
        uiStackPush(data, 3);
        if (overlayNr == OVERLAY_NR_PLAY)
        {
            enterLevel0(data);
        }
        else if (overlayNr == OVERLAY_NR_EDIT)
        {
            enterLevel4(data);
        }
        else if (overlayNr == OVERLAY_NR_SYSTEMSETTINGS)
        {
            enterLevel5(data);
        }
        else if (overlayNr == OVERLAY_NR_ABOUT)
        {
            clearSquareInt(0,0,128,43,imgBuffer);
            *strbfr=0;
            appendToString(strbfr,"About PiPicoFX");
            drawText(0,8,strbfr,imgBuffer,(void*)0);
            drawText(0,16,PI_PICO_FX_VERSION_NR,imgBuffer,(void*)0);
            *strbfr=0;
            appendToString(strbfr,"built ");
            appendToString(strbfr,PI_PICO_FX_BUILD_DATE);
            drawText(0,24,strbfr,imgBuffer,(void*)0);
            *strbfr=0;
            appendToString(strbfr,"      ");
            appendToString(strbfr,PI_PICO_FX_BUILD_TIME);
            drawText(0,32,strbfr,imgBuffer,(void*)0);

        }
        else if (overlayNr == OVERLAY_NR_FWUPDATE)
        {
            drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
            DisplayImageStandardAdressing(0,0,128,8,imgBuffer->data);
            reset_usb_boot(1 << 17,2);
        }
    }

}

static void exitCallback(PiPicoFxUiType*data)
{
    char strbfr[24];
    char nrbfr[8];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    BwImageType* imgBuffer = getImageBuffer();
    // remove overlay menu (if there)
    if (overlayNr != 0xFF)
    {
        clearSquare(0.0f,0.0f,128.0f,43.0f,imgBuffer);
        *(strbfr) = 0;
        appendToString(strbfr,"Bank:");
        UInt8ToChar(currentBank,nrbfr);
        appendToString(strbfr,nrbfr);
        drawText(5,21,strbfr,imgBuffer,font);
        *(strbfr) = 0;
        appendToString(strbfr,presets[currentPreset].name);
        drawText(5,42,strbfr,imgBuffer,font);
        overlayNr=0xFF;
    }
    else // remove ui level switching blocker
    {
        uiStackPop(data);
    }
}



static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    // change overlay icon (if there)
    if (overlayNr != 0xFF)
    {
        if (encoderDelta > 0)
        {
            overlayNr++;
            if (overlayNr > 4)
            {
                overlayNr=4;
            }
        }
        else
        {
            overlayNr--;
            if (overlayNr > 4)
            {
                overlayNr=0;
            }

        }
        drawImage(41,0,overlays[overlayNr],imgBuffer);
    }
    else // change preset
    {
        if (encoderDelta > 0 && currentPreset < 2)
        {
            handlePresetChange(BANK_PRESET_CHANGE_INCREASE,data);
        }
        else if (encoderDelta < 0 && currentPreset > 0)
        {
            handlePresetChange(BANK_PRESET_CHANGE_DECREASE,data);
        }
    }     
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
    uint8_t nbStompSwitch;
    if (bankChanged == 0)
    {
        nbStompSwitch=getStompSwitchState(1);
        if ((nbStompSwitch & 0x1) == 0x1)
        {
            handleBankChange(BANK_PRESET_CHANGE_DECREASE,data);
        }
        else if (currentPreset != 0)
        {
            setPresetNr(0,data);
            create(data);
        }
    }
    else
    {
        bankChanged = 0;
    }
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
    uint8_t nbStompSwitch1, nbStompSwitch3;
    if (bankChanged == 0)
    {
        nbStompSwitch1=getStompSwitchState(0);
        nbStompSwitch3=getStompSwitchState(2);
        if (((nbStompSwitch1 & 0x1) == 0x1) && ((nbStompSwitch3 & 0x1) == 0x0))
        {
            handleBankChange(BANK_PRESET_CHANGE_DECREASE,data);
        }
        else if (((nbStompSwitch1 & 0x1) == 0x0) && ((nbStompSwitch3 & 0x1) == 0x1))
        {
            handleBankChange(BANK_PRESET_CHANGE_INCREASE,data);
        }
        else if (currentPreset != 1)
        {
            setPresetNr(1,data);
            create(data);
        }
    }
    else
    {
        bankChanged = 0;
    }
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
    uint8_t nbStompSwitch;
    if (bankChanged == 0)
    {
        nbStompSwitch=getStompSwitchState(1);
        if ((nbStompSwitch & 0x1) == 0x1)
        {
            handleBankChange(BANK_PRESET_CHANGE_INCREASE,data);
        }
        else if (currentPreset != 2)
        {
            setPresetNr(2,data);
            create(data);
        }
    }
    else
    {
        bankChanged = 0;
    }
}

static void knob0Callback(uint16_t val, PiPicoFxUiType*data)
{
    pot1Val = val;
}

static void knob1Callback(uint16_t val, PiPicoFxUiType*data)
{
    pot2Val = val;
}

static void knob2Callback(uint16_t val, PiPicoFxUiType*data)
{
    pot3Val = val;
}

void enterLevel3(PiPicoFxUiType*data)
{
    if (loadPreset(presets,currentBank*3)!=0)
    {
        generateEmptyPreset(presets,currentBank,0);
    }
    if (loadPreset(presets+1,currentBank*3+1)!=0)
    {
        generateEmptyPreset(presets+1,currentBank,1);
    }
    if (loadPreset(presets+2, currentBank*3+2)!=0)
    {
        generateEmptyPreset(presets+2,currentBank,2);
    }
    data->editViaRotary = 1;
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
    overlayNr=0xFF;
    if (data->currentProgramIdx != presets[currentPreset].programNr)
    {
        data->currentProgramIdx = presets[currentPreset].programNr;
        programToInitialize=data->currentProgramIdx;
        programChangeState = 1;
    }
    setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
}

static void handleBankChange(uint8_t increase, PiPicoFxUiType* data)
{
    bankChanged = 1;
    if (increase)
    {
        currentBank++;
    }
    else
    {
        currentBank--;
    }
    if (currentBank > 31)
    {
        currentBank = 0;
    }
    if (loadPreset(presets,currentBank*3)!=0)
    {
        generateEmptyPreset(presets,currentBank,0);
    }
    if (loadPreset(presets+1,currentBank*3+1)!=0)
    {
        generateEmptyPreset(presets+1,currentBank,1);
    }
    if (loadPreset(presets+2, currentBank*3+2)!=0)
    {
        generateEmptyPreset(presets+2,currentBank,2);
    }
    // move preset and current program index away to force reload
    currentPreset = 0xFF;
    data->currentProgramIdx = 0xff;
    
}

static void handlePresetChange(uint8_t increase, PiPicoFxUiType*data)
{
    if (increase==1)
    {
        currentPreset++;
    }
    else if (increase == 0)
    {
        currentPreset--;
    }
    setPreset(data);
}

static void setPresetNr(uint8_t nr,PiPicoFxUiType* data)
{
    currentPreset = nr;
    setPreset(data);
}

static void setPreset(PiPicoFxUiType*data)
{
    if (data->currentProgramIdx != presets[currentPreset].programNr)
    {
        data->currentProgramIdx = presets[currentPreset].programNr;
        programToInitialize=data->currentProgramIdx;
        programChangeState=1;
    }
    else
    {
        applyPreset(presets + currentPreset,data->currentProgram);
    }
    setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
    create(data);
}

