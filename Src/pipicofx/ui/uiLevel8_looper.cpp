extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "pipicofx/pipicofxui.h"
#include "pipicofx/fxPrograms.h"
#include "audio/looper.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
}

#define COLOR_LOOPER_RECORDING 2
#define COLOR_LOOPER_OVERDUBBING 3
#define COLOR_LOOPER_PLAYING 1
#define COLOR_LOOPER_OFF 0

#define LOOP_SQUARE_CENTER 40
extern LooperDataType looper;

static void create(PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    const GFXfont * font = getGFXFont(FREEMONO12PT7B);
    drawText(4,20,"Looper",imgBuffer,font);
    if (looper.looperState == LOOPER_STATE_PLAYING)
    {
        setStompswitchColorRaw(COLOR_LOOPER_PLAYING); // should be green
    }
    else if (looper.looperState == LOOPER_STATE_OVERDUBBING)
    {
        setStompswitchColorRaw(COLOR_LOOPER_OVERDUBBING); // should be orange
    }
    else if (looper.looperState == LOOPER_STATE_RECORDING)
    {
        setStompswitchColorRaw(COLOR_LOOPER_RECORDING); // should be red
    }
    else
    {
        setStompswitchColorRaw(COLOR_LOOPER_OFF);
    }

    if (looper.indexEnd != LOOPER_INDEX_NULL)
    {
        drawSquareInt(0,LOOP_SQUARE_CENTER-5,128,LOOP_SQUARE_CENTER+5,imgBuffer);
        clearSquareInt(1,LOOP_SQUARE_CENTER-4,127,LOOP_SQUARE_CENTER+4,imgBuffer);
    }

}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    
    if (looper.indexEnd != LOOPER_INDEX_NULL)
    {
        BwImageType* imgBuffer = getImageBuffer();
        drawSquareInt(0,LOOP_SQUARE_CENTER-5,128,LOOP_SQUARE_CENTER+5,imgBuffer);
        clearSquareInt(1,LOOP_SQUARE_CENTER-4,127,LOOP_SQUARE_CENTER+4,imgBuffer);
        
        uint32_t looperContentLength = looper.indexEnd - looper.indexStart;
        if (looperContentLength != 0)
        {
            uint32_t screenIndex = ((looper.currentPosition - looper.indexStart)*126)/looperContentLength;
            if (screenIndex > 126)
            {
                screenIndex = 126;
            }
            drawSquareInt(1,LOOP_SQUARE_CENTER-4,1+(uint8_t)screenIndex,LOOP_SQUARE_CENTER+4,imgBuffer);
        }
            
    }
}

// no happy knobbing here so far
/*
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
*/

// toggles record and overdub, start playing if content present and stopped
static void stompswitch1Callback(PiPicoFxUiType* data)
{
    if (looper.indexEnd == LOOPER_INDEX_NULL && looper.looperState != LOOPER_STATE_RECORDING)
    {
        LooperStartRecording(&looper);
        setStompswitchColor(0,COLOR_LOOPER_RECORDING);
    }
    else if (looper.looperState == LOOPER_STATE_RECORDING)
    {
        LooperStopRecording(&looper);
        setStompswitchColor(0,COLOR_LOOPER_PLAYING);
    }
    else if (looper.looperState == LOOPER_STATE_OVERDUBBING)
    {
        looper.looperState = LOOPER_STATE_PLAYING;
        setStompswitchColor(0,COLOR_LOOPER_PLAYING);
    }
    else if (looper.looperState == LOOPER_STATE_PLAYING)
    {
        looper.looperState = LOOPER_STATE_OVERDUBBING;
        setStompswitchColor(0,COLOR_LOOPER_OVERDUBBING);
    }
    else
    {
        LooperStartPlaying(&looper);
        setStompswitchColor(0,COLOR_LOOPER_PLAYING);
    }
}

// do more funky stuff such as retrigger, reverse, slow down, cut etc.
static void stompswitch2Callback(PiPicoFxUiType* data)
{
    // so far retrigger
    if (looper.looperState == LOOPER_STATE_PLAYING || looper.looperState == LOOPER_STATE_OVERDUBBING)
    {
        looper.currentPosition = looper.indexStart;
    }
}

// stop and clear/delete (if stopped and recorded material present)
static void stompswitch3Callback(PiPicoFxUiType* data)
{
    if (looper.looperState != LOOPER_STATE_STOPPED)
    {
        LooperStop(&looper);
        setStompswitchColor(0,COLOR_LOOPER_OFF);
    }
    else
    {
        LooperDelete(&looper);
        BwImageType* imgBuffer = getImageBuffer();
        clearSquareInt(0,LOOP_SQUARE_CENTER-5,128,LOOP_SQUARE_CENTER+5,imgBuffer);
    }
}


/*
register exit, rotary, knobs and stompswitch callbacks
remove enter callback
register onUpdate, on Create
*/
void enterLevel8(PiPicoFxUiType*data)
{
    clearCallbackAssignments();
    registerStompswitch1PressedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompswitch2Callback);
    registerStompswitch3PressedCallback(&stompswitch3Callback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}