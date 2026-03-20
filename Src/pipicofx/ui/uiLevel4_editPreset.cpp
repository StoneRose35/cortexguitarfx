
extern "C" {
#include "stdlib.h"
#include "graphics/bwgraphics.h"
#include "graphics/gfxfont.h"
#include "drivers/oled_display.h"
#include "drivers/adc.h"
#include "pipicofx/pipicofxui.h"
#include "images/editOverlay.h"
#include "images/settingsOverlay.h"
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/stompswitches.h"
}
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/MultiAudioProcessor.hpp"

#define EDITLEVEL_ROUTING 0
#define EDITLEVEL_LEDCOLOR 1
#define EDITLEVEL_PARAMETERS 2
#define EDITLEVEL_NAME 3

#define EXIT_PRESSED_FIRST_TIME 0
#define EXIT_PRESSED_SAVE 1
#define EXIT_PRESSED_REVERT 2

extern FxPresetType presets[3];
extern uint8_t currentBank;
extern uint8_t currentPreset;
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];
extern PiPicoFXUiType ui;
extern MultiAudioProcessor audioProcessor; 

static uint8_t editType; // 0: Program
                                  // 1: Led Color
                                  // 2: Parameters 
static volatile uint8_t exitState=0; // 0: Exit pressed the first time, 1: save and exit, 2: revert and exit

static void create()
{
    char strbfr[24];
    const GFXfont * font = getGFXFont(FREESANS12PT7B);
    BwImageType * imgBuffer = getImageBuffer();
    clearImage(imgBuffer);
    *(strbfr) = 0;    
    appendToString(strbfr,presets[currentPreset].name);
    drawText(5,21,strbfr,imgBuffer,font);
    switch (editType)
    {
        case EDITLEVEL_ROUTING:
            *(strbfr) = 0;    
            appendToString(strbfr,"Routing");
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


static void enterCallback() 
{
    
    switch (editType)
    {
        case EDITLEVEL_ROUTING:
            //TODO change
            uiStackPush(4);
            (&ui)->locked = 0;
            enterLevel0();
            break;
        case EDITLEVEL_LEDCOLOR:
            presets[currentPreset].ledColorPreset++;
            presets[currentPreset].ledColorPreset &= 0x3;
            if (presets[currentPreset].ledColorPreset == 0)
            {
                presets[currentPreset].ledColorPreset++;
            }
            setStompswitchColorRaw(presets[currentPreset].ledColorPreset << (currentPreset << 1));
            break;
        case EDITLEVEL_PARAMETERS:
            uiStackPush(4);
            (&ui)->locked = 1;
            (&ui)->currentParameterIdx = 0;
            (&ui)->currentParameter = ((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameter((&ui)->currentParameterIdx);
            if (((FxProgram*)audioProcessor.getFxProgram(ui.currentProgramPosition))->getParameterCount() > 0)
            {
                enterLevel1();
            }
            break;
        case EDITLEVEL_NAME:
            uiStackPush(4);
            presets[currentPreset].name[15]=0;
            (&ui)->data = presets[currentPreset].name;
            enterLevel6();

    }
}

static void exitCallback()
{
    if (exitState == EXIT_PRESSED_FIRST_TIME)
    {
        uiStackPush(0xFF);
        exitState=1;
        create();
    }
    else if (exitState == EXIT_PRESSED_SAVE)
    {
        uiStackPop();
        savePreset(presets+currentPreset,currentBank*3 + currentPreset);
        exitState = 0;
    }
    else if (exitState == EXIT_PRESSED_REVERT)
    {
        uiStackPop();
        if (loadPreset(presets+currentPreset,currentBank*3 + currentPreset)!=0)
        {
            generateEmptyPreset(presets+currentPreset,currentBank,currentPreset);
        }
        if (programsToInitialize[0] != presets[currentPreset].programNrA || 
            programsToInitialize[1] != presets[currentPreset].programNrB ||
            programsToInitialize[2] != presets[currentPreset].programNrC)
        {
            programChangeState = 1;
        }
        exitState =  EXIT_PRESSED_FIRST_TIME;   
    }
}

static void rotaryCallback(int16_t encoderDelta)
{
    if (exitState == EXIT_PRESSED_FIRST_TIME)
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
        create();
    }
    else
    {
        if (encoderDelta > 0 && exitState == EXIT_PRESSED_SAVE)
        {
            exitState = EXIT_PRESSED_REVERT;
            create();
        }
        else if (encoderDelta < 0 && exitState == EXIT_PRESSED_REVERT)
        {
            exitState = EXIT_PRESSED_SAVE;
            create();
        }
    }
}



void enterLevel4()
{
    editType = 0;
    clearCallbackAssignments();
    registerEnterButtonPressedCallback(&enterCallback);
    registerExitButtonPressedCallback(&exitCallback);
    registerRotaryCallback(&rotaryCallback);
    registerOnCreateCallback(&create);
    create();
}

