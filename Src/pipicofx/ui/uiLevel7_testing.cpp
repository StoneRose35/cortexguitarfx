extern "C"
{
#include "pipicofx/pipicofxui.h"
#include "graphics/bwgraphics.h"
#include "drivers/display128x64.h"
#include "stdlib.h"
#include "core1Main.h"
#include "drivers/gpio.h"
#include "drivers/stompswitches.h"
#include "images/testing.h"
#include "drivers/debugLed.h"
}

static void create(PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    data->data = malloc(3);
    *((uint8_t*)data->data)=0; // stomp leds and clipping leds  state
    *((uint8_t*)data->data+1)=63; // encoder value
    *((uint8_t*)data->data+2) = 10; // horizontal position of the test image, goes from 2 to 115 
    uiStackPush(data,0xFF); // prohibiting to enter another level
    drawSquareInt(0,24,*((uint8_t*)data->data+1),32-3,imgBuffer);
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt(0,32,128,64,imgBuffer);
    *((uint8_t*)data->data+2) += 1;
    if (*((uint8_t*)data->data+2)>115)
    {
        *((uint8_t*)data->data+2) = 2;
    }
    drawImage(*((uint8_t*)data->data+2),40,&testing_streamimg,imgBuffer);
    DisplayWriteFramebufferAsync(imgBuffer->data);
}

static void enterCallback(PiPicoFxUiType*data) 
{
    uint8_t * ledState = ((uint8_t*)data->data);
    setPin(CLIPPING_LED_INPUT,*ledState & 0x1);
    *ledState ^= 0x1;
}

static void exitCallback(PiPicoFxUiType*data)
{
    uint8_t * ledState = ((uint8_t*)data->data);
    setPin(CLIPPING_LED_OUTPUT,(*ledState & 0x2) >> 1);
    *ledState ^= 0x2;
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    *((uint8_t*)data->data+1) += encoderDelta;
    if (*((uint8_t*)data->data+1) < 0 && encoderDelta < 0)
    {
        *((uint8_t*)data->data+1) = 0x0;
    }
    else if (*((uint8_t*)data->data+1) < 0 && encoderDelta > 0)
    {
        *((uint8_t*)data->data+1) = 0x7f;
    }
    clearSquareInt(0,24,127,32-3,imgBuffer);
    drawSquareInt(0,24,*((uint8_t*)data->data+1),32-3,imgBuffer);
}

static void knob0Callback(uint16_t val,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt((val >> 5)+1,0,128,8-3,imgBuffer);
    drawSquareInt(0,0,val >> 5,8-3,imgBuffer);
}

static void knob1Callback(uint16_t val,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt((val >> 5)+1,8,128,16-3,imgBuffer);
    drawSquareInt(0,8,val >> 5,16-3,imgBuffer);
}

static void knob2Callback(uint16_t val,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearSquareInt((val >> 5)+1,16,128,24-3,imgBuffer);
    drawSquareInt(0,16,val >> 5,24-3,imgBuffer);
}

static void stompswitch1Callback(PiPicoFxUiType* data)
{
    uint8_t * ledColor = (uint8_t*)data->data;
    uint8_t changedBits = ((((*ledColor >> 2)&0x3) + 1)&0x3) << 2;
    *ledColor &= ~(0x3 << 2);
    *ledColor |= changedBits; 
    setStompswitchColorRaw(*ledColor >> 2);
}

static void stompswitch2Callback(PiPicoFxUiType* data)
{
    uint8_t * ledColor = (uint8_t*)data->data;
    uint8_t changedBits = ((((*ledColor >> 4)&0x3) + 1)&0x3) << 4; 
    *ledColor &= ~(0x3 << 4);
    *ledColor |= changedBits; 
    setStompswitchColorRaw(*ledColor >> 2);
}

static void stompswitch3Callback(PiPicoFxUiType* data)
{
    uint8_t * ledColor = (uint8_t*)data->data;
    uint8_t changedBits = ((((*ledColor >> 6)&0x3) + 1)&0x3) << 6; 
    *ledColor &= ~(0x3 << 6);
    *ledColor |= changedBits; 
    setStompswitchColorRaw(*ledColor >> 2);
}

void enterLevel7(PiPicoFxUiType*data)
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
    create(data);
}