extern "C" {
#include "stdlib.h"
#include "globalConfig.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/saveOverlay.h"
#include "images/editOverlay.h"
#include "images/copyoverlay.h"
#include "images/swapoverlay.h"
#include "images/deleteoverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/aboutoverlay.h"
#include "images/fwupdateScreen.h"
#include "images/looperOverlay.h"
#include "images/ledcoloroverlay.h"
#include "images/toggleswitch_on.h"
#include "images/toggleswitch_off.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
#include "gen/version.h"
#include "bootloader_activation.h"
#include "pcm3060.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/systick.h"
}
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/MultiAudioProcessor.hpp"
#include "pipicofx/uiLevel3_preset.hpp"


/**
 * Mode "Presets": The Footswitches allow to select or switch between presets consisting of three programs
 * the preset are organized in banks
 * Left+Middle Footswitch: Bank down, load preview of next lower bank, hitting a single footswitch then selects a preset of that bank
 * Right+Middle Footswitch: Bank up, load preview of next higher bank, hitting a single footswitch then selects a preset of that bank
 * Enter: shows overlay allowing to jump to various submodes
 * Rotary+Enter: Change Mode
 */
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 
extern FxPresetType presets[3];
extern volatile uint16_t initialKnobValues[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static uint8_t overlayNr=0xFF;
static uint8_t bankChanged=0; // flag indicating that the bank has been changed upon stomp switch release
static uint8_t editOverlayMode=0;
static uint8_t copySwapBank;
static uint8_t copySwapPreset;
static FxPresetType previewPresets[3];            
static const BwImageTypeConst* overlays[]={
    &saveOverlay_streamimg,
    &editOverlay_streamimg,
    &ledcoloroverlay_streamimg,
    &copyoverlay_streamimg,
    &swapoverlay_streamimg,
    &deleteoverlay_streamimg,  
    &settingsOverlay_streamimg, 
    &aboutoverlay_streamimg, 
    &fwUpgradeOverlay_streamimg};
extern volatile uint8_t programsToInitialize[3]; 
extern volatile uint8_t programChangeState;

static uint8_t presetChangeLock = 0; // used to prohibit action when the second stomp switch is released
static uint8_t currentVolume;
static uint8_t previewBankNr=0xFF;
static uint32_t longPressTickStart = 0;
static uint8_t handleReleaseEvent = 0;
static uint8_t channelState=0xf; // bits 0-1 HiZ, bits2-3 mic, 0: off  1: on, 3: unknown
static void handleBankChange(uint8_t);
static void handlePresetChange(uint8_t);
static void createBankPreviewOverlay(uint8_t bankNr,BwImageType*img);
static void limitPreviewBankRange(uint8_t increase);
static void createPresetSelector(BwImageType*imgBuffer);
//static void drawParameterDisplay(FxProgram*prog,BwImageStruct*imgBuffer);
static void drawBankAndPreset(BwImageStruct*imgBuffer);
//static void drawPrograms(BwImageType* imgBuffer);
static void reloadPresetsFromEeprom(FxPresetType*priis,uint8_t bnk);

#define BANK_PRESET_CHANGE_NONE 2
#define BANK_PRESET_CHANGE_INCREASE 1
#define BANK_PRESET_CHANGE_DECREASE 0 

#define LVL3_OVERLAY_NR_SAVE 0
#define LVL3_OVERLAY_NR_EDIT_NAME 1
#define LVL3_OVERLAY_NR_EDIT_LED_COLOR 2
#define LVL3_OVERLAY_NR_COPY 3
#define LVL3_OVERLAY_NR_SWAP 4
#define LVL3_OVERLAY_NR_DELETE 5
#define LVL3_OVERLAY_NR_SYSTEMSETTINGS 6
#define LVL3_OVERLAY_NR_ABOUT 7
#define LVL3_OVERLAY_NR_FWUPDATE 8


#define EOM_NONE 0
#define EOM_OVERLAYS 1
#define EOM_COPY 2
#define EOM_SWAP 3
#define EOM_DELETE 4
#define EOM_COPY_COMMIT 5
#define EOM_SWAP_COMMIT 6
#define EOM_DELETE_COMMIT 7
#define EOM_ABOUT 8
#define EOM_FIRMWARE_UPDATE 9
#define EOM_SAVE 10
#define EOM_BANKPREVIEW 11

#define BANK_LIMIT 32


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
    drawBankAndPreset(imgBuffer);
    drawBottomPanel(&bottomPane);
    drawImage(0,48,(BwImageTypeConst*)&bottomPane,imgBuffer);


    if (editOverlayMode == EOM_OVERLAYS)
    {
        drawImage(41,0,overlays[overlayNr],imgBuffer);
    }
    else if (editOverlayMode == EOM_SAVE)
    {
        clearSquareInt(64-35,4,64+35,22,imgBuffer);
        drawRectFrame(64-35,4,64+35,22,imgBuffer);
        drawText(64-35+2,4+2+8,"Enter:Save",imgBuffer,0);
        drawText(64-35+2,4+2+16,"Exit:Revert",imgBuffer,0);
    }
    else if (editOverlayMode == EOM_ABOUT)
    {
        drawAbout(imgBuffer);
    }
    else if (editOverlayMode == EOM_FIRMWARE_UPDATE)
    {
        drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
    }
    else if (editOverlayMode == EOM_COPY || editOverlayMode == EOM_SWAP)
    {
        createPresetSelector(imgBuffer);
    }
    else if (editOverlayMode == EOM_DELETE_COMMIT || editOverlayMode == EOM_COPY_COMMIT || editOverlayMode == EOM_SWAP_COMMIT )
    {
        clearSquareInt(64-32,4,64+32,24,imgBuffer);
        drawRectFrame(64-32,4,64+32,24,imgBuffer);
        drawText(64-32+2,4+2+7,"Enter:OK",imgBuffer,(void*)0);
        drawText(64-32+2,4+2+8+7,"Exit:Abort",imgBuffer,(void*)0);
    }
    else if (editOverlayMode == EOM_BANKPREVIEW)
    {
        BwImageType previewImage=
        {
            .data=(uint8_t*)malloc(96*48/8),
            .sx=96,
            .sy=48,
            .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
            .byteSize=96*48/8
        };
        createBankPreviewOverlay(previewBankNr,&previewImage);
        drawImage(2,2,(BwImageTypeConst*)&previewImage,imgBuffer);
        free((void*)previewImage.data);
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
    


    if (longPressTickStart != 0 and getTickValue() - longPressTickStart > LONGPRESS_DURATION_SYSTICKS)
    {
        longPressTickStart = 0;
        uiStackPush(3);
        enterLevel8();
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
    // show overlay menu (if not there)
    switch (editOverlayMode)
    {
        case EOM_NONE:
            editOverlayMode = EOM_OVERLAYS;
            overlayNr = LVL3_OVERLAY_NR_SAVE;
            uiStackPush(0xFF);
            break;
        case EOM_OVERLAYS:
            if (overlayNr == LVL3_OVERLAY_NR_SAVE)
            {
                editOverlayMode = EOM_SAVE;
            }
            if (overlayNr == LVL3_OVERLAY_NR_EDIT_NAME)
            {
                editOverlayMode = EOM_NONE;
                uiStackPop();
                uiStackPush(3);   
                presets[currentPreset].name[15]=0;
                ui.data = presets[currentPreset].name;
                enterLevel6();
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
                editOverlayMode = EOM_ABOUT;
            }
            else if (overlayNr == LVL3_OVERLAY_NR_FWUPDATE)
            {
                editOverlayMode = EOM_FIRMWARE_UPDATE;
            }
            else if (overlayNr == LVL3_OVERLAY_NR_COPY)
            {
                editOverlayMode = EOM_COPY;
                copySwapPreset = currentPreset;
                copySwapBank = currentBank;
                reloadPresetsFromEeprom(previewPresets,copySwapBank);
            }
            else if (overlayNr == LVL3_OVERLAY_NR_SWAP)
            {
                editOverlayMode = EOM_SWAP;
                copySwapPreset = currentPreset;
                copySwapBank = currentBank;
                reloadPresetsFromEeprom(previewPresets,copySwapBank);
            }
            else if (overlayNr == LVL3_OVERLAY_NR_DELETE)
            {
                editOverlayMode = EOM_DELETE_COMMIT;
            }
            else if (overlayNr == LVL3_OVERLAY_NR_EDIT_LED_COLOR)
            {
                presets[currentPreset].ledColorPreset++;
                presets[currentPreset].ledColorPreset &= 0x3;
                if (presets[currentPreset].ledColorPreset == 0)
                {
                    presets[currentPreset].ledColorPreset++;
                }
                setStompswitchColorRaw(presets[currentPreset].ledColorPreset << (currentPreset << 1));
            }
            break;
        case EOM_COPY:
            editOverlayMode = EOM_COPY_COMMIT;
            break;
        case EOM_SWAP:
            editOverlayMode = EOM_SWAP_COMMIT;
            break;
        case EOM_DELETE:
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
            setPreset();
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            break;
        case EOM_DELETE_COMMIT:
            clearPreset(currentBank*3+currentPreset);
            generateEmptyPreset(presets + currentPreset,currentBank,currentPreset);
            setPreset();
            break;
        case EOM_SAVE:
            editOverlayMode = EOM_NONE;
            parametersToPreset(presets+currentPreset,&audioProcessor);
            savePreset(presets+currentPreset,currentBank*3 + currentPreset);
            uiStackPop();
            uiStackPush(3);
            break;    
    }
}

static void exitCallback()
{
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
        case EOM_ABOUT:
        case EOM_FIRMWARE_UPDATE:
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            break;
        case EOM_COPY:
        case EOM_SWAP:
            editOverlayMode = EOM_OVERLAYS;
            break;
        case EOM_SAVE:
            overlayNr=0xFF;
            editOverlayMode = EOM_NONE;
            uiStackPop();
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
            uiStackPush(3);
            break;
        default:
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
    BwImageType* imgBuffer = getImageBuffer();
    // change overlay icon (if there)
    switch (editOverlayMode)
    {
        case EOM_OVERLAYS:
            if (encoderDelta > 0)
            {
                overlayNr++;
                if (overlayNr >= sizeof(overlays)/(sizeof(BwImageTypeConst*)))
                {
                    overlayNr=sizeof(overlays)/(sizeof(BwImageTypeConst*))-1;
                }
            }
            else
            {
                overlayNr--;
                if (overlayNr >= sizeof(overlays)/(sizeof(BwImageTypeConst*)))
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
                reloadPresetsFromEeprom(previewPresets,copySwapBank);
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
    if (handleReleaseEvent == 0 || (editOverlayMode != EOM_NONE && editOverlayMode != EOM_BANKPREVIEW))
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
            setPresetNr(0);
            editOverlayMode = EOM_NONE;
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
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
    if (handleReleaseEvent == 0 || (editOverlayMode != EOM_NONE && editOverlayMode != EOM_BANKPREVIEW))
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
            setPresetNr(1);
            editOverlayMode = EOM_NONE;
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
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
    if (handleReleaseEvent == 0 || (editOverlayMode != EOM_NONE && editOverlayMode != EOM_BANKPREVIEW))
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
            setPresetNr(2);
            editOverlayMode = EOM_NONE;
        }
        else if (presetChangeLock == 1)
        {
            presetChangeLock = 0;
        }
    }
    else
    {
        bankChanged = 0;
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
            else if (val <= 2047 && (channelState & 0x3)!=0)
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
            else if (val <= 2047 && ((channelState>>2) & 0x3)!=0 )
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

void enterLevel3()
{
    ui.editViaRotary = 1;
    clearCallbackAssignments();
    registerEnterButtonReleasedCallback(&enterReleasedCallback);
    registerEnterButtonPressedCallback(&enterPressedCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerLeftButtonPressedCallback(&leftCallback);
    registerRightButtonPressedCallback(&rightCallback);
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
    //overlayNr=0xFF;
    handleReleaseEvent = 0;
    ui.defaultOn=1;
    channelState = 0xF;
    switch (ui.currentProgramPosition)
    {
        case 0:
            ui.currentProgramIdx = presets[currentPreset].programNrA;
            break;
        case 1:
            ui.currentProgramIdx = presets[currentPreset].programNrB;
            break;
        case 2:
            ui.currentProgramIdx = presets[currentPreset].programNrC;
            break;    
        default:
            break;    
    }

    setStompswitchColorRaw(presets[currentPreset].ledColorPreset << (currentPreset << 1));
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
    reloadPresetsFromEeprom(previewPresets,previewBankNr);
    editOverlayMode = EOM_BANKPREVIEW;
    presetChangeLock = 1;

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

    setPreset();
}

void setPresetAtBank(uint8_t bankNr,uint8_t presetNr)
{
    previewBankNr = bankNr;
    setPresetNr(presetNr);
}

void setPresetNr(uint8_t nr)
{
    if (previewBankNr != currentBank && previewBankNr != 0xFF)
    {
        currentBank = previewBankNr;
        previewBankNr = 0xFF;
        reloadPresetsFromEeprom(presets,currentBank);
    }
    currentPreset = nr;
    setPreset();
}

void setPreset()
{
    if (presets[currentPreset].programNrA != 0x3F && (audioProcessor.getFxProgram(0) == nullptr || 
        (audioProcessor.getFxProgram(0) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(0))->getIndex() != presets[currentPreset].programNrA)))
    {
        programsToInitialize[0] = presets[currentPreset].programNrA | 0x80;
    }
    else if (presets[currentPreset].programNrA == 0x3F) // no effect should be set
    {
        programsToInitialize[0]=0x7e;
    }
    else // effect program remains unchanged, only apply parameters
    {
        programsToInitialize[0]=0x7f | 0x80;
    }

    if (presets[currentPreset].programNrB != 0x3F && (audioProcessor.getFxProgram(1) == nullptr || 
        (audioProcessor.getFxProgram(1) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(1))->getIndex() != presets[currentPreset].programNrB)))
    {
        programsToInitialize[1] = presets[currentPreset].programNrB | 0x80;
    }
    else if (presets[currentPreset].programNrB == 0x3F)
    {
        programsToInitialize[1]=0x7e;
    }
    else
    {
        programsToInitialize[0]=0x7f | 0x80;
    }

    if (presets[currentPreset].programNrC != 0x3F && (audioProcessor.getFxProgram(2) == nullptr || 
        (audioProcessor.getFxProgram(2) != nullptr && 
        ((FxProgram*)audioProcessor.getFxProgram(2))->getIndex() != presets[currentPreset].programNrC)))
    {
        programsToInitialize[2] = presets[currentPreset].programNrC | 0x80;
    }
    else if (presets[currentPreset].programNrC == 0x3F)
    {
        programsToInitialize[2]=0x7e;
    }
    else
    {
        programsToInitialize[2]=0x7f | 0x80;
    }

    programChangeState = 1;
    setStompswitchColorRaw(presets[currentPreset].ledColorPreset << (currentPreset << 1));
    //create();
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
    
    uint8_t textLineData[90];
    BwImageStruct textLine={
        .data=textLineData,
        .sx=90,
        .sy=8,
        .type=BWIMAGE_BW_IMAGE_STRUCT_VERTICAL_BYTES,
        .byteSize=90
    };
    // preset names and rectangles around them
    clearImage(&textLine);
    drawText(0,1+7,previewPresets[0].name,&textLine,(void*)0);
    drawImage(10+4,4+13+0*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+0*13,106-2,4+10+12+0*13,imgBuffer);
    clearImage(&textLine);
    drawText(0,1+7,previewPresets[1].name,&textLine,(void*)0);
    drawImage(10+4,4+13+1*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+1*13,106-2,4+10+12+1*13,imgBuffer);
    clearImage(&textLine);
    drawText(0,1+7,previewPresets[2].name,&textLine,(void*)0);
    drawImage(10+4,4+13+2*13,(BwImageStructConst*)&textLine,imgBuffer);
    drawRectFrame(10+2,4+10+2+2*13,106-2,4+10+12+2*13,imgBuffer);
    // bold rectangle around selected preset
    drawRectFrame(10+1,4+10+1+copySwapPreset*13,106-1,4+10+13+copySwapPreset*13,imgBuffer);
}
static void createBankPreviewOverlay(uint8_t bankNr,BwImageType*img)
{
    char bfr[32];
    char nrBfr[8];
    clearSquareInt(0,0,96-1,48-1,img);
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

static void drawBankAndPreset(BwImageStruct*imgBuffer)
{
    char strbfr[24];
    char nrbfr[4];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    *(strbfr) = 0;
    appendToString(strbfr,presets[currentPreset].name);
    drawText(2,18,strbfr,imgBuffer,font);
    *(strbfr) = 0;
    appendToString(strbfr,"Bank ");
    UInt8ToChar(currentBank,nrbfr);
    appendToString(strbfr,nrbfr);
    drawText(2,34,strbfr,imgBuffer,getGFXFont(FREESANSBOLD9PT7B));

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

