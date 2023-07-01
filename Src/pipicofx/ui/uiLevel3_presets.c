#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "images/fwUpgradeOverlay.h"
#include "images/fwupdateScreen.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "stompswitches.h"

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static volatile uint8_t overlayNr=0xFF;
static volatile uint8_t bankChanged=0; // flag indicating that the bank has been changed upon stomp switch release
                                       // used to prohob action when the second stomp switch is released
const BwImageType* overlays[]={&editOverlay_streamimg, &settingsOverlay_streamimg, &fwUpgradeOverlay_streamimg};
extern volatile uint8_t programsToInitialize[3];
extern volatile uint8_t programChangeState;

#define OVERLAY_NR_EDIT 0
#define OVERLAY_NR_SYSTEMSETTINGS 1
#define OVERLAY_NR_FWUPDATE 2

static void create(PiPicoFxUiType*data)
{
    char strbfr[8];
    char nrbfr[8];
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

    *(strbfr) = 0;
    appendToString(strbfr,"In");
    font = getGFXFont(FREEMONO12PT7B);
    drawText(5,42+10,strbfr,imgBuffer,(void*)0);

    *(strbfr) = 0;
    appendToString(strbfr,"Out");
    drawText(5,42+20,strbfr,imgBuffer,(void*)0);

    applyPreset(presets+currentPreset,fxPrograms);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{    
    BwImageType* imgBuffer = getImageBuffer();
    // draw Level bars
    clearSquare(40.0f,43.0f,128.0f,64.0f,imgBuffer);
    //in
    drawSquare(40.0f,43.0f,40.0f + int2float(avgInput)*(128.0f-40.0f)/128.0f,52.0f,imgBuffer);
    //out
    drawSquare(40.0f,53.0f,40.0f + int2float(avgOutput)*(128.0f-40.0f)/128.0f,64.0f,imgBuffer);

    ssd1306writeFramebufferAsync(imgBuffer->data);
}

static void enterCallback(PiPicoFxUiType*data) 
{
    BwImageType* imgBuffer = getImageBuffer();
    // show overlay menu (if not there)
    if (overlayNr == 0xFF)
    {
        overlayNr = OVERLAY_NR_EDIT;
        drawImage(41,0,&editOverlay_streamimg,imgBuffer);
        uiStackPush(data,0xFF);
    }
    else
    {
        uiStackPop(data);
        uiStackPush(data, 3);
        if (overlayNr == OVERLAY_NR_EDIT)
        {
            enterLevel4(data);
        }
        else if (overlayNr == OVERLAY_NR_SYSTEMSETTINGS)
        {
            enterLevel5(data);
        }
        else if (overlayNr == OVERLAY_NR_FWUPDATE)
        {
            drawImage(0,0,&fwupdateScreen_streamimg,imgBuffer);
            ssd1306DisplayImageStandardAdressing(0,0,128,8,imgBuffer->data);
            reset_usb_boot(0,0);
        }
    }

}

static void exitCallback(PiPicoFxUiType*data)
{
    char strbfr[8];
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
            if (overlayNr > 2)
            {
                overlayNr=1;
            }
        }
        else
        {
            overlayNr--;
            if (overlayNr > 2)
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
            currentPreset++;
        }
        else if (encoderDelta < 0 && currentPreset > 0)
        {
            currentPreset--;
        }
        if (data->currentProgramIdx != presets[currentPreset].programNr)
        {
            programsToInitialize[0]=presets[currentPreset].programNr;
            programChangeState=1;
            //data->currentProgram = fxPrograms[data->currentProgramIdx];
            //data->currentParameterIdx=0;
            //data->currentParameter = data->currentProgram->parameters;
        }
        setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
        applyPreset(presets + currentPreset,fxPrograms);
        create(data);
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
            bankChanged = 1;
            currentBank--;
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
        }
        else
        {
            if (currentPreset != 0)
            {
                currentPreset = 0;
                if (data->currentProgramIdx != presets[currentPreset].programNr)
                {
                    programsToInitialize[0]=presets[currentPreset].programNr;
                    programChangeState = 1;
                }
                setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
                applyPreset(presets,fxPrograms);    
            }
        }
        create(data);
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
            bankChanged = 1;
            currentBank--;
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
        }
        else if (((nbStompSwitch1 & 0x1) == 0x0) && ((nbStompSwitch3 & 0x1) == 0x1))
        {
            bankChanged = 1;
            currentBank++;
            if (currentBank > 31)
            {
                currentBank = 31;
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
        }
        else
        {
            if (currentPreset != 1)
            {
                currentPreset = 1;
                if (data->currentProgramIdx != presets[currentPreset].programNr)
                {
                    programsToInitialize[0]=presets[currentPreset].programNr;
                    programChangeState = 1;
                }
                setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
                applyPreset(presets,fxPrograms);
            }
        }
        create(data);
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
            bankChanged = 1;
            currentBank++;
            if (currentBank > 31)
            {
                currentBank = 31;
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
        }
        else
        {
            if (currentPreset != 2)
            {
                currentPreset = 2;
                if (data->currentProgramIdx != presets[currentPreset].programNr)
                {
                    programsToInitialize[0]=presets[currentPreset].programNr;
                    programChangeState = 1;
                }
                setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
                applyPreset(presets,fxPrograms);
            }
        }
        create(data);
    }
    else
    {
        bankChanged = 0;
    }
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
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
    overlayNr=0xFF;
    if (data->currentProgramIdx != presets[currentPreset].programNr)
    {
        programsToInitialize[0]=presets[currentPreset].programNr;
        programChangeState = 1;
    }
    setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
    applyPreset(presets, fxPrograms);
}

