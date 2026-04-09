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
extern PiPicoFXUiType ui;
static void create()
{    
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
    const GFXfont * font = getGFXFont(FREEMONO12PT7B);
    drawText(4,20,"Looper",imgBuffer,font);
    drawBottomPanel(&bottomPane);
    drawImage(0,48,(BwImageTypeConst*)&bottomPane,imgBuffer);
    if (looper.indexEnd != LOOPER_INDEX_NULL)
    {
        
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
    clearSquareInt(6,32-7,127,32,imgBuffer);
    if (looper.looperFunction == LOOPER_FUNCTION_RETRIGGER)
    {
        drawText(6,32,"middle: retrigger",imgBuffer,(void*)0);
    }
    else if (looper.looperFunction == LOOPER_FUNCTION_EXIT)
    {
        drawText(6,32,"middle: exit",imgBuffer,(void*)0);
    }
}

// no happy knobbing here so far
/*
static void knob0Callback(uint16_t val,PiPicoFXUiType*data)
{
}

static void knob1Callback(uint16_t val,PiPicoFXUiType*data)
{
}

static void knob2Callback(uint16_t val,PiPicoFXUiType*data)
{
}

static void enterCallback(PiPicoFXUiType*data) 
{
}

static void exitCallback(PiPicoFXUiType*data)
{
}
*/
static void rotaryCallback(int16_t encoderDelta)
{
    if (ui.enterState == 1)
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
    else
    {
        if (looper.looperFunction == LOOPER_FUNCTION_RETRIGGER)
        {
            looper.looperFunction = LOOPER_FUNCTION_EXIT;
        }
        else
        {
            looper.looperFunction = LOOPER_FUNCTION_RETRIGGER;
        }
    }
}


// toggles record and overdub, start playing if content present and stopped
static void stompswitch1Callback()
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
static void stompswitch2Callback()
{
    // retrigger
    if ((looper.looperState == LOOPER_STATE_PLAYING || looper.looperState == LOOPER_STATE_OVERDUBBING) && looper.looperFunction == LOOPER_FUNCTION_RETRIGGER)
    {
        looper.currentPosition = looper.indexStart;
    }
    else if (looper.looperFunction == LOOPER_FUNCTION_EXIT) // exit to last screen
    {
        if(uiStackCurrent() != 0xFF)
        {
            getEnterFunctions()[uiStackPop()]();
        }
    }
}

// stop and clear/delete (if stopped and recorded material present)
static void stompswitch3Callback(void)
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

/*
register exit, rotary, knobs and stompswitch callbacks
remove enter callback
register onUpdate, on Create
*/
void enterLevel8()
{
    //clearCallbackAssignments();
    registerStompswitch1ReleasedCallback(0);
    registerStompswitch2ReleasedCallback(0);
    registerStompswitch3ReleasedCallback(0);
    registerStompswitch1PressedCallback(&stompswitch1Callback);
    registerStompswitch2PressedCallback(&stompswitch2Callback);
    registerStompswitch3PressedCallback(&stompswitch3Callback);
    registerLeftButtonPressedCallback(&leftCallback);
    registerRightButtonPressedCallback(&rightCallback);
    registerRotaryCallback(&rotaryCallback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create();
}