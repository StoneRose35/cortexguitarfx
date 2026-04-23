extern "C" {
#include "./../Inc/graphics/bwgraphics.h"
#include "./../Inc/fonts/oled_font_5x7.h"
#include "pipicofx/pipicofxui.h"
#include "stringFunctions.h"
#include "audio/looper.h"
#include "sai.h"
#include "audioEngine.h"
}
#include "pipicofx/FxProgramLoader.hpp"
#include "inc/bmplib.hpp"
#include <iostream>
#include <string>
PiPicoFXUiType ui;
MultiAudioProcessor audioProcessor; 
uint8_t programsToInitialize[3];
const uint8_t stompswitch_progs[]={8,7,1};
volatile uint16_t initialKnobValues[3];
volatile uint8_t programChangeState;
volatile uint16_t audioState;
volatile uint16_t audioTransferState;
volatile float avgOutOld,avgInOld;
volatile uint32_t cpuLoad;
FxPresetType presets[3];
uint8_t currentBank;
uint8_t currentPreset;
LooperDataType looper;
static BwImageBufferType imgBuffer;
static uint32_t pseudoCnt=0;
float looperData[32];
static int32_t audioBufferIn[64];
static int32_t audioBufferOut[64];
uint32_t task;
uint32_t sampleCnt=0;
uint8_t mockedStompSwitchStates[3]={0,0,0};
/** mocked hardware methods */
extern "C" {
uint32_t getTickValue()
{
    return sampleCnt/480; // simulates a new tick every 480 samples / 10ms
}

uint16_t getChannel0Value()
{
 return 796;
}

uint16_t getChannel1Value()
{
 return 2048;
}

uint16_t getChannel2Value()
{
 return 3255;
}

void setStompswitchColorRaw(uint8_t data)
{
    std::cout << "setting stomp switch colors to " << std::__cxx11::to_string(data) << std::endl;
}

float convolve(const float*coeffs,float*data,uint32_t offset)
{
    float res=0.0f;
    for(uint32_t c=0;c<64;c++)
    {
        res += *(coeffs + c) * *(data + ((offset + c)&0x3F));
    }
    return res;
}

void pcm3060SetOutputVolume(uint8_t channel,uint8_t volume)
{
    std::cout << "setting volume of channel " << std::__cxx11::to_string(channel) << " to " << std::__cxx11::to_string(volume) << std::endl;
}


void pcm3060SetInputState(uint8_t channel, uint8_t val)
{
    std::cout << " setting channel " << std::__cxx11::to_string(channel) << " to value " << std::__cxx11::to_string(val) << std::endl; 
}

uint8_t getStompSwitchState(uint8_t switchNr)
{
    return mockedStompSwitchStates[switchNr];
}

//void enterLevel0(void){}
void enterLevel1(void){}
void enterLevel2(void){}
//void enterLevel3(void){}
void enterLevel4(void){}
void enterLevel5(void){}
void enterLevel6(void){}
void enterLevel7(void){}
void enterLevel8(void){}
void enterLevel9(void){}
void enterLevel10(void){}
void enterLevel11(void){}

uint32_t getTimeLW()
{
    pseudoCnt =+ 100;
    return pseudoCnt;
}

int32_t *  getEditableAudioBufferHiRes()
{
    return audioBufferOut;
}

int32_t * getInputAudioBufferHiRes()
{
    return audioBufferIn;
}

}


void mockProgramChange()
{
    AudioProcessor * currentFxProgram;
    // remove all entries and replaces them by new one 
    // when programsToInitialize is not 0x3f at the given position
    // updates the currently edited parameter of the ui structure
    if (programChangeState == 3)
    {         
        for (uint8_t q = 0;q < 3;q++)
        {   


            if ((programsToInitialize[q] & 0x7F) != 0x7F)
            {
                currentFxProgram = audioProcessor.removeFxProgram(q);
                if (currentFxProgram != nullptr)
                {
                    if (q == ui.currentProgramPosition)
                    {
                        ui.currentProgram = 0;
                        ui.currentParameter = 0;
                        ui.currentParameterIdx = 0;
                    }
                    delete currentFxProgram; 
                    currentFxProgram = nullptr;
                }

                currentFxProgram = loadProgram(programsToInitialize[q]&0x7F);
                audioProcessor.addFxProgram(currentFxProgram,q);
                if (currentFxProgram != nullptr)
                {
                    if (ui.defaultOn)
                    {
                        ((FxProgram*)currentFxProgram)->switchOn();
                    }
                    else
                    {
                        ((FxProgram*)currentFxProgram)->switchOff();
                    }
                    
                    ui.currentParameterIdx = 0;
                    if (q == ui.currentProgramPosition)
                    {
                        ui.currentProgram = ((FxProgram*)currentFxProgram);
                        ui.currentProgramIdx = programsToInitialize[q]&0x7F;
                        ui.currentParameter = ((FxProgram*)currentFxProgram)->getParameter(ui.currentParameterIdx);
                    }

                    if (programsToInitialize[q] & 0x80)
                    {
                        applyPresetToProgram(presets+currentPreset,&audioProcessor,q);
                    }
                }
                //onCreate();
            }
        }
        //applyPreset(presets+currentPreset,&audioProcessor);
        programChangeState = 4;
    }
}

void setupMockedUi(void)
{
    BwImageType* img = getImageBuffer();
    ui.currentProgramPosition = 0;
    ui.currentProgramIdx=presets->programNrA;
    ui.currentParameterIdx=0;
    ui.locked=0;
    ui.editViaRotary =0;
    ui.uiLevelStackPtr = 0;
    *(ui.uiLevelStack) = 0;
    for (uint8_t c=1;c<8;c++)
    {
        *(ui.uiLevelStack + c) = 0xFF;
    }
    imgBuffer.sx=128;
    imgBuffer.sy=64;
    img->byteSize = (imgBuffer.sx*imgBuffer.sy)>>3;
    img->data = imgBuffer.data;
    img->sx = imgBuffer.sx;
    img->sy = imgBuffer.sy;
    img->type = imgBuffer.type;
    looper.memoryPointer = looperData;
}

int main(int argc,char** argv)
{
    char * demoPresetName="TheManual";
    BitmapFileHeaderType bmpHeader;
    initBmpFile(&bmpHeader,64*4,128*4);
    piPicoFxUiSetup();
    initAudioEngine();
    LooperInit(&looper);
    
    enterLevel3();
    uint8_t cnt=0;
    while (*(demoPresetName+ cnt))
    {
        presets[0].name[cnt] = *(demoPresetName+cnt);
        cnt++;
    }
    presets[0].name[cnt] = 0;
    avgInOld = 0.001f;
    avgOutOld = 0.001f;
    // process 512 sample to virtually load the program
    for (uint16_t c=0;c<512;c++)
    {
        processAudioBuffers();
        sampleCnt++;
        mockProgramChange();
    }
    cpuLoad = 133;
    onUpdate(0,0,0);
    BwImageType* img = getImageBuffer();
    renderImage(img,&bmpHeader,4);
    writeBmp("uiExample.bmp",&bmpHeader);
}