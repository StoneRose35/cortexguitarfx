#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "romfunc.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"

#define EDITLEVEL_PROGRAM 0
#define EDITLEVEL_LEDCOLOR 1
#define EDITLEVEL_PARAMETERS 2
#define EDITLEVEL_NAME 3

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern volatile uint8_t programsToInitialize[3];
extern volatile uint8_t programChangeState;

static volatile uint8_t editType; // 0: Program
                                  // 1: Led Color
                                  // 2: Parameters 
static volatile uint8_t exitState=0; // 0: Exit pressed the first time, 1: save and exit, 2: revert and exit

static void create(PiPicoFxUiType*data)
{
    char strbfr[8];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    BwImageType * imgBuffer = getImageBuffer();
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
        case EDITLEVEL_NAME:
            *(strbfr) = 0;    
            appendToString(strbfr,"Edit Name");
            break;    
    }
    font = getGFXFont(FREESANS9PT7B);
    drawText(5,42,strbfr,imgBuffer,font);
    switch (exitState)
    {
        case 1:
            *strbfr = 0;
            appendToString(strbfr,"Save? Yes");
            drawText(5,60,strbfr,imgBuffer,font);
            break;
        case 2:
            *strbfr = 0;
            appendToString(strbfr,"Save? No");
            drawText(5,60,strbfr,imgBuffer,font);
            break;
        default:
            break;
    }
}

static void update(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad,PiPicoFxUiType*data)
{
    BwImageType* imgBuffer = getImageBuffer();
    OledwriteFramebufferAsync(imgBuffer->data);
}


static void enterCallback(PiPicoFxUiType*data) 
{
    
    switch (editType)
    {
        case EDITLEVEL_PROGRAM:
            uiStackPush(data,4);
            data->locked = 1;
            enterLevel0(data);
            break;
        case EDITLEVEL_LEDCOLOR:
            presets[currentPreset].ledColor++;
            presets[currentPreset].ledColor &= 0x3;
            if (presets[currentPreset].ledColor == 0)
            {
                presets[currentPreset].ledColor++;
            }
            setStompswitchColorRaw(presets[currentPreset].ledColor << (currentPreset << 1));
            break;
        case EDITLEVEL_PARAMETERS:
            uiStackPush(data,4);
            data->locked = 1;
            data->currentParameterIdx = 0;
            data->currentParameter = data->currentProgram->parameters + data->currentParameterIdx;
            if (data->currentProgram->nParameters > 0)
            {
                enterLevel1(data);
            }
            break;
        case EDITLEVEL_NAME:
            uiStackPush(data,4);
            presets[currentPreset].name[23]=0;
            data->data = presets[currentPreset].name;
            enterLevel6(data);

    }
}

static void exitCallback(PiPicoFxUiType*data)
{
    if (exitState == 0)
    {
        uiStackPush(data,0xFF);
        exitState=1;
        create(data);
    }
    else if (exitState == 1)
    {
        uiStackPop(data);
        savePreset(presets+currentPreset,currentBank*3 + currentPreset);
        exitState = 0;
    }
    else if (exitState == 2)
    {
        uiStackPop(data);
        if (loadPreset(presets+currentPreset,currentBank*3 + currentPreset)!=0)
        {
            generateEmptyPreset(presets+currentPreset,currentBank,currentPreset);
        }
        if (data->currentProgramIdx != presets[currentPreset].programNr)
        {
            programsToInitialize[0] = presets[currentPreset].programNr;
            programChangeState = 1;
        }
        applyPreset(presets+currentPreset,fxPrograms);
        exitState =  0;   
    }
}

static void rotaryCallback(int16_t encoderDelta,PiPicoFxUiType*data)
{
    if (exitState == 0)
    {
        if (encoderDelta > 0)
        {
            editType++;
            if (editType > 3)
            {
                editType = 3;
            }
        }
        else if (encoderDelta < 0)
        {
            editType--;
            if (editType > 3)
            {
                editType = 0;
            }
        }
        create(data);
    }
    else
    {
        if (encoderDelta > 0 && exitState == 1)
        {
            exitState = 2;
            create(data);
        }
        else if (encoderDelta < 0 && exitState == 2)
        {
            exitState = 1;
            create(data);
        }
    }
}



void enterLevel4(PiPicoFxUiType*data)
{
    editType = 0;
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerOnUpdateCallback(&update);
    registerOnCreateCallback(&create);
    create(data);
}

