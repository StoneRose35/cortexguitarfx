extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/display128x64.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
}

static void create()
{
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    (void)avgInput;
    (void)avgOutput;
    (void)cpuLoad;
}


static void knob0Callback(uint16_t val)
{
    (void)val;
}

static void knob1Callback(uint16_t val)
{
    (void)val;
}

static void knob2Callback(uint16_t val)
{
    (void)val;
}

static void enterCallback() 
{
}

static void exitCallback()
{
}

static void rotaryCallback(int16_t encoderDelta)
{
    (void)encoderDelta;
}

static void stompswitch1Callback()
{
}

static void stompswitch2Callback()
{
}

static void stompswitch3Callback()
{
}

void enterLevel99()
{
    clearCallbackAssignments();
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
    create();
}

