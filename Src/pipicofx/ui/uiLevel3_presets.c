#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
static volatile uint8_t overlayNr=0xFF;
const BwImageType* overlays[]={&editOverlay_streamimg, &settingsOverlay_streamimg};
extern volatile uint8_t programsToInitialize[3];

static void create(PiPicoFxUiType*data)
{
    char strbfr[8];
    char nrbfr[8];
    BwImageBufferType* imgBuffer = getImageBuffer();
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
    font = getGFXFont(FREESANS9PT7B);
    drawText(5,42+10,strbfr,imgBuffer,font);

    *(strbfr) = 0;
    appendToString(strbfr,"Out");
    drawText(5,42+20,strbfr,imgBuffer,font);

    applyPreset(presets+currentPreset,fxPrograms);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{    
    BwImageBufferType* imgBuffer = getImageBuffer();
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
    BwImageBufferType* imgBuffer = getImageBuffer();
    // show overlay menu (if not there)
    if (overlayNr == 0xFF)
    {
        overlayNr = 0;
        drawImage(41,0,&editOverlay_streamimg,imgBuffer);
    }
    else
    {
        data->lastUiLevel = 3;
        //enterLevel4(data);
    // move to: edit preset, system settings if overlay there
    }

}

static void exitCallback(PiPicoFxUiType*data)
{
    char strbfr[8];
    char nrbfr[8];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    BwImageBufferType* imgBuffer = getImageBuffer();
    // remove overlay menu (if there)
    if (overlayNr != 0xFF)
    {
        clearSquare(0.0f,0.0f,128.0f,42.0f,imgBuffer);
        *(strbfr) = 0;
        appendToString(strbfr,"Bank:");
        UInt8ToChar(currentBank,nrbfr);
        appendToString(strbfr,nrbfr);
        drawText(5,42,strbfr,imgBuffer,font);
        *(strbfr) = 0;
        appendToString(strbfr,presets[currentPreset].name);
        drawText(5,42,strbfr,imgBuffer,font);
        overlayNr=0xFF;
    }
}

static void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
    // nothing
}

static void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
    // nothing
}

static void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
    // nothing
}


static void rotaryCallback(uint16_t encoderDelta,PiPicoFxUiType*data)
{
    uint8_t oldPresetProgram;
    BwImageBufferType* imgBuffer = getImageBuffer();
    // change overlay icon (if there)
    if (overlayNr != 0xFF)
    {
        if (encoderDelta > 0)
        {
            overlayNr++;
            if (overlayNr > 1)
            {
                overlayNr=1;
            }
        }
        else
        {
            overlayNr--;
            if (overlayNr > 1)
            {
                overlayNr=0;
            }

        }
        drawImage(41,0,overlays[overlayNr],imgBuffer);
    }
    else // change preset
    {
        oldPresetProgram = presets[currentPreset].programNr;
        if (encoderDelta > 0 && currentPreset < 2)
        {
            currentPreset++;
        }
        else if (encoderDelta < 0 && currentPreset > 0)
        {
            currentPreset--;
        }
        if (oldPresetProgram != presets[currentPreset].programNr)
        {
            programsToInitialize[0]=presets[currentPreset].programNr;
        }
        else
        {
            applyPreset(presets + currentPreset,fxPrograms);
        }
    }     
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
    // if switch 2 momentarily pressed: decrease Bank

    // else: switch to preset 1 if not already chosen
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
    // if switch 1 momentarily pressed: decrease Bank

    // if switch 3 momentarily pressed: increase Bank

    // else: switch to preset 2 if not already chosen
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
    // if switch 2 momentarily pressed: increase Bank

    // else: switch to preset 3 if not already chosen
}

void enterLevel3(PiPicoFxUiType*data)
{
    loadPreset(presets,currentBank*3);
    loadPreset(presets+1,currentBank*3+1);
    loadPreset(presets+2, currentBank*3+2);
    data->editViaRotary = 1;

    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerStompswitch1ReleasedCallback(&stompswitch1Callback);
    registerStompswitch2ReleasedCallback(&stompswitch2Callback);
    registerStompswitch3ReleasedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

