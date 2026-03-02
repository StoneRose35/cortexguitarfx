extern "C" {
#include "stdlib.h"
#include "globalConfig.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/playoverlay.h"
#include "images/editOverlay.h"
#include "images/copyoverlay.h"
#include "images/swapoverlay.h"
#include "images/deleteoverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/fwupdateScreen.h"
#include "images/looperOverlay.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "gen/version.h"
#include "bootloader_activation.h"
#include "systick.h"
}
#include "pipicofx/FxProgramLoader.hpp"
extern PiPicoFXUiType ui;
extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static uint8_t overlayNr=0xFF;
static uint8_t bankChanged=0; // flag indicating that the bank has been changed upon stomp switch release
static uint8_t editOverlayMode=0;
static uint8_t copySwapBank;
static uint8_t copySwapPreset;
                                       
static const BwImageTypeConst* overlays[]={
    &looperOverlay_streamimg,
    &editOverlay_streamimg,
    &copyoverlay_streamimg,
    &swapoverlay_streamimg,
    &deleteoverlay_streamimg,  
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};
extern volatile uint8_t programToInitialize;
extern volatile uint8_t programChangeState;
extern volatile uint8_t consumeEnterReleased;

static uint8_t presetChangeLock = 0; // used to prohibit action when the second stomp switch is released
static uint16_t pot1Val=0;
static uint16_t pot2Val=0;
static uint16_t pot3Val=0;
static uint8_t previewBankNr=0xFF;
static uint32_t longPressTickStart = 0;
static uint8_t handleReleaseEvent = 0;
static volatile uint8_t enterState=0;
static void handleBankChange(uint8_t);
static void handlePresetChange(uint8_t);
static void setPresetNr(uint8_t,PiPicoFXUiType*);
static void setPreset(PiPicoFXUiType*);
static void createBankPreviewOverlay(uint8_t bankNr,BwImageType*img);
static void limitPreviewBankRange(uint8_t increase);
static void createPresetSelector(BwImageType*imgBuffer);
static void drawParameterDisplay(FxProgram*prog,BwImageStruct*imgBuffer);
static void drawBankAndPreset(BwImageStruct*imgBuffer);
static void reloadPresetsFromEeprom(FxPresetType*priis,uint8_t bnk);

#define BANK_PRESET_CHANGE_NONE 2
#define BANK_PRESET_CHANGE_INCREASE 1
#define BANK_PRESET_CHANGE_DECREASE 0 

#define LVL3_OVERLAY_NR_LOOPER 0
#define LVL3_OVERLAY_NR_EDIT 1
#define LVL3_OVERLAY_NR_COPY 2
#define LVL3_OVERLAY_NR_SWAP 3
#define LVL3_OVERLAY_NR_DELETE 4
#define LVL3_OVERLAY_NR_SYSTEMSETTINGS 5
#define LVL3_OVERLAY_NR_ABOUT 6
#define LVL3_OVERLAY_NR_FWUPDATE 7
#define LVL3_OVERLAY_NR_ABOUT_SHOWING 8


#define EOM_NONE 0
#define EOM_OVERLAYS 1
#define EOM_COPY 2
#define EOM_SWAP 3
#define EOM_DELETE 4
#define EOM_COPY_COMMIT 5
#define EOM_SWAP_COMMIT 6
#define EOM_DELETE_COMMIT 7

#define BANK_LIMIT 32



static void create()
{
    BwImageType* imgBuffer = getImageBuffer();
    // display preset Name and Bank Number
    clearImage(imgBuffer);
    drawBankAndPreset(imgBuffer);
    drawParameterDisplay(ui.currentProgram,imgBuffer);

    pot1Val = getChannel0Value();
    pot2Val = getChannel1Value();
    pot3Val = getChannel2Value();

    
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{    
    uint16_t yval; 
    (void)cpuLoad;
    if (editOverlayMode != EOM_NONE)
    {
        return;
    }
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

    if (longPressTickStart != 0 and getTickValue() - longPressTickStart > LONGPRESS_DURATION_SYSTICKS)
    {
        longPressTickStart = 0;
        uiStackPush(3);
        enterLevel8();
    }
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
    BwImageType* imgBuffer = getImageBuffer();
    char strbfr[24];
    // show overlay menu (if not there)
    switch (editOverlayMode)
    {
        case EOM_NONE:
            editOverlayMode = EOM_OVERLAYS;
            overlayNr = LVL3_OVERLAY_NR_LOOPER;
            drawImage(41,0,&looperOverlay_streamimg,imgBuffer);
            uiStackPush(0xFF);
            break;
        case EOM_OVERLAYS:
            if (overlayNr == LVL3_OVERLAY_NR_LOOPER)
            {
                editOverlayMode = EOM_NONE;
                uiStackPop();
                uiStackPush(3);   
                enterLevel8();
            }
            else if (overlayNr == LVL3_OVERLAY_NR_EDIT)
            {
                editOverlayMode = EOM_NONE;
                uiStackPop();
                uiStackPush(3);   
                enterLevel4();
            }
            else if (overlayNr == LVL3_OVERLAY_NR_SYSTEMSETTINGS)
            {
                editOverlayMode = EOM_NONE;
                uiStackPop();
                uiStackPush(3);   
                enterLevel5();
            }
            else if (overlayNr == LVL3_OVERLAY_NR_ABOUT)
            {
                editOverlayMode = EOM_NONE;
                overlayNr = LVL3_OVERLAY_NR_ABOUT_SHOWING;
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
            else if (overlayNr == LVL3_OVERLAY_NR_FWUPDATE)
            {
                editOverlayMode = EOM_NONE;
                drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
                DisplayImageStandardAdressing(0,0,128,8,imgBuffer->data);
                jumpToBootloader();
            }
            else if (overlayNr == LVL3_OVERLAY_NR_COPY)
            {
                editOverlayMode = EOM_COPY;
                copySwapPreset = currentPreset;
                copySwapBank = currentBank;
                createPresetSelector(imgBuffer);
            }
            else if (overlayNr == LVL3_OVERLAY_NR_SWAP)
            {
                editOverlayMode = EOM_SWAP;
                copySwapPreset = currentPreset;
                copySwapBank = currentBank;
                createPresetSelector(imgBuffer);
            }
            else if (overlayNr == LVL3_OVERLAY_NR_DELETE)
            {
                editOverlayMode = EOM_DELETE_COMMIT;
                // display question dialog
                clearSquareInt(64-32,4,64+32,24,imgBuffer);
                drawRectFrame(64-32,4,64+32,24,imgBuffer);
                drawText(64-32+2,4+2+7,"Enter:OK",imgBuffer,(void*)0);
                drawText(64-32+2,4+2+8+7,"Exit:Abort",imgBuffer,(void*)0);
                editOverlayMode = EOM_DELETE_COMMIT;
            }
            break;
        case EOM_COPY:
            // display question dialog
            clearSquareInt(64-32,4,64+32,24,imgBuffer);
            drawRectFrame(64-32,4,64+32,24,imgBuffer);
            drawText(64-32+2,4+2+7,"Enter:OK",imgBuffer,(void*)0);
            drawText(64-32+2,4+2+8+7,"Exit:Abort",imgBuffer,(void*)0);
            editOverlayMode = EOM_COPY_COMMIT;
            break;
        case EOM_SWAP:
            // display question dialog
            clearSquareInt(64-32,4,64+32,24,imgBuffer);
            drawRectFrame(64-32,4,64+32,24,imgBuffer);
            drawText(64-32+2,4+2+7,"Enter:OK",imgBuffer,(void*)0);
            drawText(64-32+2,4+2+8+7,"Exit:Abort",imgBuffer,(void*)0);
            editOverlayMode = EOM_SWAP_COMMIT;
            break;
        case EOM_DELETE:
            // display question dialog
            clearSquareInt(64-32,4,64+32,24,imgBuffer);
            drawRectFrame(64-32,4,64+32,24,imgBuffer);
            drawText(64-32+2,4+2+7,"Enter:OK",imgBuffer,(void*)0);
            drawText(64-32+2,4+2+8+7,"Exit:Abort",imgBuffer,(void*)0);
            editOverlayMode = EOM_DELETE_COMMIT;
            break;
        case EOM_COPY_COMMIT:
            FxPresetType presetToCopy;
            if (loadPreset(&presetToCopy,currentBank*3+currentPreset)==0)
            {
                savePreset(&presetToCopy,copySwapBank*3+copySwapPreset);
            }
            else // copying and empty preset results in deletion
            {
                clearPreset(copySwapBank*3+copySwapPreset);
            }
            if (copySwapBank == currentBank)
            {
                reloadPresetsFromEeprom(presets,currentBank);
            }
            // jump back to norma display
            clearSquareInt(41,0,41+47,43,imgBuffer);
            drawBankAndPreset(imgBuffer);
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            break;
        case EOM_SWAP_COMMIT:
            FxPresetType swapOrigin,swapTarget;
            uint8_t originLoadResult,targetLoadResult;
            originLoadResult = loadPreset(&swapOrigin,currentBank*3+currentPreset);
            targetLoadResult = loadPreset(&swapTarget,copySwapBank*3+copySwapPreset);
            if (originLoadResult==0)
            {
                savePreset(&swapOrigin,copySwapBank*3+copySwapPreset);
            }
            else // copying and empty preset results in deletion
            {
                clearPreset(copySwapBank*3+copySwapPreset);
            }
            if (targetLoadResult==0)
            {
                savePreset(&swapTarget,currentBank*3+currentPreset);
            }
            else
            {
                clearPreset(currentBank*3+currentPreset);
            }
            if (loadPreset(presets+currentPreset,currentBank*3+currentPreset)!=0)
            {
                generateEmptyPreset(presets+currentPreset,currentBank,currentPreset);
            }
            reloadPresetsFromEeprom(presets,currentBank);
            setPreset(&ui);
            // jump back to normal display
            clearSquareInt(41,0,41+47,43,imgBuffer);
            drawBankAndPreset(imgBuffer);
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            break;
        case EOM_DELETE_COMMIT:
            clearPreset(currentBank*3+currentPreset);
            generateEmptyPreset(presets + currentPreset,currentBank,currentPreset);
            setPreset(&ui);
            break;
    }
}

static void exitCallback()
{
    BwImageType* imgBuffer = getImageBuffer();
    // remove overlay menu (if there)
    switch (editOverlayMode)
    {
        case EOM_NONE:
            //uiStackPop(data);
            break;
        case EOM_OVERLAYS:
        case EOM_COPY_COMMIT:
        case EOM_SWAP_COMMIT:
        case EOM_DELETE_COMMIT:
            clearSquareInt(0,0,112,64,imgBuffer);
            drawBankAndPreset(imgBuffer);
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            break;
        case EOM_COPY:
        case EOM_SWAP:
            editOverlayMode = EOM_OVERLAYS;
            clearSquareInt(0,0,112,64,imgBuffer);
            drawBankAndPreset(imgBuffer);
            drawImage(41,0,overlays[overlayNr],imgBuffer);
            drawParameterDisplay(ui.currentProgram,imgBuffer);
            break;
    }
}



static void rotaryCallback(int16_t encoderDelta)
{
    if (enterState==1)
    {
        consumeEnterReleased = 1;
        enterLevel0();
        return;
    }
    BwImageType* imgBuffer = getImageBuffer();
    // change overlay icon (if there)
    switch (editOverlayMode)
    {
        case EOM_OVERLAYS:
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
        case EOM_NONE:
            if (encoderDelta > 0 && currentPreset < 2)
            {
                handlePresetChange(BANK_PRESET_CHANGE_INCREASE);
            }
            else if (encoderDelta < 0 && currentPreset > 0)
            {
                handlePresetChange(BANK_PRESET_CHANGE_DECREASE);
            }
            break;
        case EOM_COPY:
        case EOM_SWAP:
            uint8_t oldCopyPreset,oldCopyBank;
            oldCopyPreset = copySwapPreset;
            oldCopyBank = copySwapBank;
            if (encoderDelta > 0)
            {
                copySwapPreset++;
                if (copySwapPreset > 2)
                {
                    copySwapBank++;
                    copySwapPreset = 0;
                }
            }
            else
            {
                copySwapPreset--;
                if (copySwapPreset > 2)
                {
                    copySwapBank--;
                    if (copySwapBank >(BANK_LIMIT-1))
                    {
                        copySwapBank = 0;
                    }
                    copySwapPreset=2;
                }
            }
            if (copySwapBank != oldCopyBank) // Bank has changed: complete redraw
            {
                createPresetSelector(imgBuffer);
            }
            else    // only preset has changed, change only highlighted rectangle
            {
                clearRectFrame(10+1,4+10+1+oldCopyPreset*13,106-1,4+10+13+oldCopyPreset*13,imgBuffer);
                drawRectFrame(10+1,4+10+1+copySwapPreset*13,106-1,4+10+13+copySwapPreset*13,imgBuffer);
            }
            break;
    }

}

static void stompswitch1Callback(void)
{
    uint8_t nbStompSwitch;
    if (handleReleaseEvent == 0)
    {
        return;
    }
    longPressTickStart = 0;
    if (bankChanged == 0)
    {
        nbStompSwitch=getStompSwitchState(1);
        if ((nbStompSwitch & 0x1) == 0x1)
        {
            handleBankChange(BANK_PRESET_CHANGE_DECREASE);
        }
        else if ((currentPreset != 0 || previewBankNr != currentBank) && presetChangeLock == 0)
        {
            setPresetNr(0,&ui);
            create();
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
        }
        else
        {
            create();
        }
        
    }
    else
    {
        bankChanged = 0;
    }
}

static void stompswitch2Callback(void)
{
    uint8_t nbStompSwitch1, nbStompSwitch3;
    if (handleReleaseEvent == 0)
    {
        return;
    }
    longPressTickStart = 0;
    if (bankChanged == 0)
    {
        nbStompSwitch1=getStompSwitchState(0);
        nbStompSwitch3=getStompSwitchState(2);
        if (((nbStompSwitch1 & 0x1) == 0x1) && ((nbStompSwitch3 & 0x1) == 0x0))
        {
            handleBankChange(BANK_PRESET_CHANGE_DECREASE);
        }
        else if (((nbStompSwitch1 & 0x1) == 0x0) && ((nbStompSwitch3 & 0x1) == 0x1))
        {
            handleBankChange(BANK_PRESET_CHANGE_INCREASE);
        }
        else if ((currentPreset != 1 || previewBankNr != currentBank) && presetChangeLock == 0)
        {
            setPresetNr(1,&ui);
            create();
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
        }
        else
        {
            create();
        }
    }
    else
    {
        bankChanged = 0;
    }
}

static void stompswitch3Callback(void)
{
    uint8_t nbStompSwitch;
    if (handleReleaseEvent == 0)
    {
        return;
    }
    longPressTickStart = 0;
    if (bankChanged == 0)
    {
        nbStompSwitch=getStompSwitchState(1);
        if ((nbStompSwitch & 0x1) == 0x1)
        {
            handleBankChange(BANK_PRESET_CHANGE_INCREASE);
        }
        else if ((currentPreset != 2 || previewBankNr != currentBank) && presetChangeLock == 0)
        {
            setPresetNr(2,&ui);
            create();
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
        }
        else
        {
            create();
        }
    }
    else
    {
        bankChanged = 0;
    }
}

static void stompSwitch1PressedCallback()
{
    handleReleaseEvent = 1;
    if (currentPreset == 0)
    {
        longPressTickStart = getTickValue();
    }
}

static void stompSwitch2PressedCallback()
{
    handleReleaseEvent = 1;
    if (currentPreset == 1)
    {
        longPressTickStart = getTickValue();
    }
}

static void stompSwitch3PressedCallback(void)
{
    handleReleaseEvent = 1;
    if (currentPreset == 2)
    {
        longPressTickStart = getTickValue();
    }
}

static void knob0Callback(uint16_t val)
{
    pot1Val = val;
}

static void knob1Callback(uint16_t val)
{
    pot2Val = val;
}

static void knob2Callback(uint16_t val)
{
    pot3Val = val;
}

void enterLevel3()
{
    reloadPresetsFromEeprom(presets,currentBank);
    ui.editViaRotary = 1;
    clearCallbackAssignments();
    registerEnterButtonReleasedCallback(&enterReleasedCallback);
    registerEnterButtonPressedCallback(&enterPressedCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerStompswitch1PressedCallback(&stompSwitch1PressedCallback);
    registerStompswitch2PressedCallback(&stompSwitch2PressedCallback);
    registerStompswitch3PressedCallback(&stompSwitch3PressedCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create();
    overlayNr=0xFF;
    handleReleaseEvent = 0;
    enterState = 0;
    ui.defaultOn=1;

    ui.currentProgramIdx = presets[currentPreset].programNr;
    programToInitialize=ui.currentProgramIdx;
    programChangeState = 1;

    setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
}

static void handleBankChange(uint8_t increase)
{
    // display preset overlay if not there already
    if (previewBankNr == 0xFF)
    {
        if (increase)
        {
            previewBankNr = currentBank+1;
        }
        else {
            previewBankNr = currentBank-1;
        }
        limitPreviewBankRange(increase);
    }
    else
    {
        if (increase)
        {
            previewBankNr++;
        }
        else
        {
            previewBankNr--;
        }
        limitPreviewBankRange(increase);
    }
    BwImageType previewImage=
    {
        .data=(uint8_t*)malloc(96*48/8),
        .sx=96,
        .sy=48,
        .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize=96*48/8
    };
    createBankPreviewOverlay(previewBankNr,&previewImage);
    BwImageType* imgBuffer = getImageBuffer();
    drawImage(2,2,(BwImageTypeConst*)&previewImage,imgBuffer);
    free((void*)previewImage.data);
    presetChangeLock = 1;
    // move preset index away to force reload
    //currentPreset = 0xFF;
    //data->currentProgramIdx = 0xff;
}

static void handlePresetChange(uint8_t increase)
{
    if (increase==1)
    {
        currentPreset++;
    }
    else if (increase == 0)
    {
        currentPreset--;
    }
    setPreset(&ui);
}

static void setPresetNr(uint8_t nr,PiPicoFXUiType* data)
{
    if (previewBankNr != currentBank && previewBankNr != 0xFF)
    {
        currentBank = previewBankNr;
        previewBankNr = 0xFF;
        reloadPresetsFromEeprom(presets,currentBank);
    }
    currentPreset = nr;
    setPreset(data);
}

static void setPreset(PiPicoFXUiType*data)
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
    create();
}


static void createPresetSelector(BwImageType*imgBuffer)
{
    char strbfr[24];
    char nrbfr[4];
    clearSquareInt (10,4,106,62,imgBuffer);
    drawRectFrame(10,4,106,62,imgBuffer);
    *strbfr=0;
    appendToString(strbfr,"Bank ");
    *nrbfr=0;
    UInt8ToChar(copySwapBank,nrbfr);
    appendToString(strbfr,nrbfr);
    drawText(10+2,4+2+7,strbfr,imgBuffer,(void*)0);
    drawHorizontal(14,4,106,imgBuffer);
    FxPresetType copyPresets[3];
    uint8_t textLineData[90];
    BwImageStruct textLine={
        .data=textLineData,
        .sx=90,
        .sy=8,
        .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize=90
    };

    reloadPresetsFromEeprom(copyPresets,copySwapBank);
    // preset names and rectangles around them
    clearImage(&textLine);
    drawText(0,1+7,copyPresets[0].name,&textLine,(void*)0);
    drawImage(10+4,4+13+0*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+0*13,106-2,4+10+12+0*13,imgBuffer);
    clearImage(&textLine);
    drawText(0,1+7,copyPresets[1].name,&textLine,(void*)0);
    drawImage(10+4,4+13+1*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+1*13,106-2,4+10+12+1*13,imgBuffer);
    clearImage(&textLine);
    drawText(0,1+7,copyPresets[2].name,&textLine,(void*)0);
    drawImage(10+4,4+13+2*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+2*13,106-2,4+10+12+2*13,imgBuffer);
    // bold rectangle around selected preset
    drawRectFrame(10+1,4+10+1+copySwapPreset*13,106-1,4+10+13+copySwapPreset*13,imgBuffer);
}
static void createBankPreviewOverlay(uint8_t bankNr,BwImageType*img)
{
    FxPresetType previewPresets[3];
    char bfr[32];
    char nrBfr[8];
    //const GFXfont * font = getGFXFont(TOMTHUMB);
    reloadPresetsFromEeprom(previewPresets,bankNr);
    clearSquareInt(0,0,96-1,48-1,img);
    //clearSquareInt(1,1,96-1,48-1,img);
    drawRectFrame(0,0,96-1,48-1,img);
    *bfr=0;
    *nrBfr=0;
    appendToString(bfr,"Bank ");
    UInt8ToChar(bankNr,nrBfr);
    appendToString(bfr,nrBfr);
    if (bankNr == currentBank)
    {
        appendToString(bfr, " *");
    }
    drawText(3,0*10+9,bfr,img,0);
    *bfr=0;
    appendToString(bfr,previewPresets->name);
    if (bankNr == currentBank && currentPreset == 0)
    {
        appendToString(bfr, " *");
    }
    drawText(3,1*10+9,bfr,img,0);
    *bfr=0;
    appendToString(bfr,(previewPresets+1)->name);
    if (bankNr == currentBank && currentPreset == 1)
    {
        appendToString(bfr, " *");
    }
    drawText(3,2*10+9,bfr,img,0);
    *bfr=0;
    appendToString(bfr,(previewPresets+2)->name);
    if (bankNr == currentBank && currentPreset == 2)
    {
        appendToString(bfr, " *");
    }
    drawText(3,3*10+9,bfr,img,0);    
}

static void drawParameterDisplay(FxProgram*prog,BwImageStruct*imgBuffer)
{
    uint8_t startY;
    uint16_t val_p1=0xFFFF,val_p2=0xFFFF,val_p3=0xFFFF;
    clearSquareInt(128-4-2*6,0,128,64,imgBuffer);
    for (uint8_t c=0;c<prog->getParameterCount();c++)
    {
        uint8_t currentCtrl = prog->getParameter(c)->getControl();
        switch (currentCtrl)
        {
        case 0:
            val_p1 = prog->getParameter(c)->rawValue;
            break;
        case 1:
            val_p2 = prog->getParameter(c)->rawValue;
            break;
        case 2: 
            val_p3 = prog->getParameter(c)->rawValue;
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
        // value of p3
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
        // value of p1
        drawSquareInt(128-4+1-2*6,startY,128-1-2*6,64-1,imgBuffer);
    }
}

static void drawBankAndPreset(BwImageStruct*imgBuffer)
{
    char strbfr[24];
    char nrbfr[4];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    *(strbfr) = 0;
    appendToString(strbfr,"Bank:");
    UInt8ToChar(currentBank,nrbfr);
    appendToString(strbfr,nrbfr);
    drawText(5,21,strbfr,imgBuffer,font);
    *(strbfr) = 0;
    appendToString(strbfr,presets[currentPreset].name);
    drawText(5,42,strbfr,imgBuffer,font);
}

static void reloadPresetsFromEeprom(FxPresetType*priis,uint8_t bnk)
{
    if (loadPreset(priis,bnk*3)!=0)
    {
        generateEmptyPreset(priis,bnk,0);
    }
    if (loadPreset(priis+1,bnk*3+1)!=0)
    {
        generateEmptyPreset(priis+1,bnk,1);
    }
    if (loadPreset(priis+2, bnk*3+2)!=0)
    {
        generateEmptyPreset(priis+2,bnk,2);
    }
}

static void limitPreviewBankRange(uint8_t increase)
{
    if (previewBankNr > (BANK_LIMIT-1))
    {
        if (increase==0)
        {
            previewBankNr = 0;
        }
        else
        {
            previewBankNr =BANK_LIMIT-1;
        }
    }
}

