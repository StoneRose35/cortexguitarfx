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

#define EDITLEVEL_PROGRAM 0
#define EDITLEVEL_LEDCOLOR 1
#define EDITLEVEL_PARAMETERS 2

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;

static volatile uint8_t editType; // 0: Program
                                  // 1: Led Color
                                  // 2: Parameters 

static void create(PiPicoFxUiType*data)
{
    char strbfr[8];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    BwImageBufferType * imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    *(strbfr) = 0;    
    appendToString(strbfr,presets[currentPreset].name);
    drawText(5,21,strbfr,imgBuffer,font);
    switch (editType)
    {
        case EDITLEVEL_PROGRAM:
            *(strbfr) = 0;    
            appendToString(strbfr,"Select Program");
            break;
        case EDITLEVEL_LEDCOLOR:
            *(strbfr) = 0;    
            appendToString(strbfr,"Led Color");
            break;
        case EDITLEVEL_PARAMETERS:
            *(strbfr) = 0;    
            appendToString(strbfr,"Edit Params");
            break;       
    }
    font = getGFXFont(FREESANS9PT7B);
    drawText(5,42,strbfr,imgBuffer,font);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageBufferType* imgBuffer = getImageBuffer();
    ssd1306writeFramebufferAsync(imgBuffer->data);
}

static void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
}

static void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
}

static void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
}

static void enterCallback(PiPicoFxUiType*data) 
{
    
    switch (editType)
    {
        case EDITLEVEL_PROGRAM:
            data->lastUiLevel = 4;
            enterLevel0(data);
            break;
        case EDITLEVEL_LEDCOLOR:

            break;
        case EDITLEVEL_PARAMETERS:
            data->lastUiLevel = 4;
            enterLevel2(data);
            break;
    }
}

static void exitCallback(PiPicoFxUiType*data)
{
}

static void rotaryCallback(uint16_t encoderDelta,PiPicoFxUiType*data)
{
    char strbfr[8];
    const GFXfont * font;
    BwImageBufferType * imgBuffer = getImageBuffer();
    if (encoderDelta > 0)
    {
        editType++;
        if (editType > 2)
        {
            editType = 2;
        }
    }
    else if (encoderDelta < 0)
    {
        editType++;
        if (editType > 2)
        {
            editType = 0;
        }
    }
    clearSquare(0.f,22.f,128.f,42.f,imgBuffer);
    switch (editType)
    {
        case EDITLEVEL_PROGRAM:
            *(strbfr) = 0;    
            appendToString(strbfr,"Select Program");
            break;
        case EDITLEVEL_LEDCOLOR:
            *(strbfr) = 0;    
            appendToString(strbfr,"Led Color");
            break;
        case EDITLEVEL_PARAMETERS:
            *(strbfr) = 0;    
            appendToString(strbfr,"Edit Params");
            break;       
    }
    font = getGFXFont(FREESANS9PT7B);
    drawText(5,42,strbfr,imgBuffer,font);
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
}

void enterLevel4(PiPicoFxUiType*data)
{
    editType = 0;
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerKnob0Callback(&knob0Callback);
    registerKnob1Callback(&knob1Callback);
    registerKnob2Callback(&knob2Callback);
    registerStompswitch1PressedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompswitch2Callback);
    registerStompswitch3PressedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

