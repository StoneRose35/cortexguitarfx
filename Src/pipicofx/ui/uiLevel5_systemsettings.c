#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "ssd1306_display.h"
#include "adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static void create(PiPicoFxUiType*data)
{
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
}

static void exitCallback(PiPicoFxUiType*data)
{
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
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

void enterLevel5(PiPicoFxUiType*data)
{
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

