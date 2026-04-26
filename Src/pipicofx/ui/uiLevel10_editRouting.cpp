extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "images/topo_2p_s.h"
#include "images/topo_2s_p.h"
#include "images/topo_parallel.h"
#include "images/topo_s_2p.h"
#include "images/topo_serial.h"
}
#include "pipicofx/MultiAudioProcessor.hpp"

extern MultiAudioProcessor audioProcessor;
extern PiPicoFXUiType ui;
extern volatile uint8_t programsToInitialize[3];

volatile static uint8_t editProgramPosition=0;
static void create()
{
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    switch (audioProcessor.getRouting())
    {
        case MULTI_AUDIO_PROCESSOR_ROUTING_3P:
            drawImage(128-48,0,&topo_parallel_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_3S:
            drawImage(128-48,0,&topo_serial_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_S_2P:
            drawImage(128-48,0,&topo_s_2p_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2P_S:
            drawImage(128-48,0,&topo_2p_s_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2S_P:
            drawImage(128-48,0,&topo_2s_p_streamimg,imgBuffer);
            break;
        default:
            break;
    }
    
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad)
{
    BwImageType* imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    switch (audioProcessor.getRouting())
    {
        case MULTI_AUDIO_PROCESSOR_ROUTING_3P:
            drawImage(64-24,0,&topo_parallel_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_3S:
            drawImage(64-24,0,&topo_serial_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_S_2P:
            drawImage(64-24,0,&topo_s_2p_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2P_S:
            drawImage(64-24,0,&topo_2p_s_streamimg,imgBuffer);
            break;
        case MULTI_AUDIO_PROCESSOR_ROUTING_2S_P:
            drawImage(64-24,0,&topo_2s_p_streamimg,imgBuffer);
            break;
        default:
            break;
    }
    /*
    *strbfr = 0;
    appendToString(strbfr,"Prog:");
    switch (ui.currentProgramPosition)
    {
        case 0:
            appendToString(strbfr,"A");
            break;
        case 1:
            appendToString(strbfr,"B");
            break;
        case 2:
            appendToString(strbfr,"C");
            break;
        default:
            break;
    }
    drawText(4,4+8,strbfr,imgBuffer,nullptr);
    */
    if (editProgramPosition)
    {
        drawHorizontal(14,4,30,imgBuffer);
    }
    else
    {
        drawHorizontal(50,64-24,64+24,imgBuffer);
    }

        // draw Level bars
    clearSquareInt(0,54,128-3*6,64,imgBuffer);
    //in
    drawSquareInt(0,54,0 + ((avgInput)*(128-3*6))/128,56,imgBuffer);
    //out
    drawSquareInt(0,58,0 + ((avgOutput)*(128-3*6))/128,60,imgBuffer);
    //cpu load
    drawSquareInt(0,62,0 + ((cpuLoad)*(128-3*6))/128,64,imgBuffer);
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
    //editProgramPosition ^= 1;
}

static void exitCallback()
{
}

static void rotaryCallback(int16_t encoderDelta)
{
    if (editProgramPosition)
    {
        if (encoderDelta > 0)
        {
            programsToInitialize[ui.currentProgramPosition] = 0x7F;
            ui.currentProgramPosition++;
            if (ui.currentProgramPosition > 2)
            {
                ui.currentProgramPosition = 2;
            }
        }
        else
        {
            programsToInitialize[ui.currentProgramPosition] = 0x7F;
            ui.currentProgramPosition--;
            if (ui.currentProgramPosition > 2)
            {
                ui.currentProgramPosition = 0;
            }
        }
    }
    else
    {
        if (encoderDelta > 0)
        {
            audioProcessor.setRouting(audioProcessor.getRouting() + 1);
        }
        else
        {
            audioProcessor.setRouting(audioProcessor.getRouting() - 1);
        }
    }
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

void enterLevel10()
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

