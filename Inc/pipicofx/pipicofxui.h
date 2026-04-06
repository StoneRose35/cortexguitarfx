#ifndef _PI_PICO_FX_UI_H_
#define _PI_PICO_FX_UI_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "pipicofx/fxPrograms.h"
#include "graphics/bwgraphics.h"
#ifdef __cplusplus
}
#endif
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/fxProgramParameter.hpp"

#define PIPICOFX_UI_STACK_SIZE 8

typedef struct 
{
    PiPicoFX::FxProgram * currentProgram; // the fx program / single effect currently in focus
    PiPicoFX::FxProgramParameter * currentParameter; // the parameter currently in focus
    uint8_t uiLevelStack[8]; //visited page numbers to indicated the menu page we're currently in
    void * data; // generic data to hand over from one menu page to another one, used to report the preset name from the string editor 
    uint8_t uiLevelStackPtr; // internal: the pointer to the current position  within uiLevelStack
    uint8_t currentProgramIdx; // the index of the program currently in focus in the ist of programs
    uint8_t currentParameterIdx; // the index of the parameter currently i focus within the list of parameters of an FxProgram
    uint8_t locked : 1; // if set: analog pot should not change effect settings
    uint8_t editViaRotary : 1; // if set: all parameters must be edited using the rotary encoder
    uint8_t defaultOn : 1; // switches programs on when changing programs, off otherwise 
    uint8_t currentProgramPosition: 2; // is either 0:A, 1:B or 2:C
    uint8_t mode : 3; // defines the mode which determined which aspects are shown and cap be manipulated
    uint8_t enterState; // reflects the state of the enter button, 1 is pressed and 0 is released
    uint8_t bypassEnterReleased; // indicated if the released event of the enter button should be discarded, this is the case if enter has been used as a modifier
} PiPicoFXUiType;

/* mode change: enter+rotary, change of enter released if rotary
   position changed while enter depressed
   exit: back to previous mode 
*/

/*
Upper part Display: Preset Name, Bank and Preset Nr
stompswitches select presets within bank, left+middle stompswitch: bank down
long press on stomp switch: jump to looper mode
middle+right stompswitch: bank up 
overlays: save/reset,edit name, edit led color, copy preset, swap preset, delete preset, settings,  about, firmware upgrade
enter+p1: switch on/off HiZ, enter+p2: switch on/off Mic, enter+p3: set global volume
*/
#define PPFX_MODE_PRESETS 0

/*
stompswitches switch on/off/freeze individual stompboxes of the preset
Upper part Display:
Program of A, Program of B, Program of C
enter+stomp switch: set focus to stompbox and enter stompbox mode
overlays: save/reset, routing, settings, about, firmware upgrade
enter+p1: switch on/off HiZ, enter+p2: switch on/off Mic, enter+p3: set global volume
*/
#define PPFX_MODE_PEDALBOARD 1


/*
left stomp, right stomp,rotary: select next/previous effect
Upper part Display: Program/Effect name 
p1 parameter and value
p2 parameter and value
p3 parameter and value
parameter in focus
enter+left/right stomp: change parameter in focus
enter+p1: lock/unlock parameters, reset when locking
enter+p3: set global volume
p1,p2,p3: parameter edit (if unlocked)
middle stompbox: switch on/off/freeze effect
overlays: save/reset, edit parameter, about, settings, firmware upgrade
*/
#define PPFX_MODE_STOMPBOX 2


/*
left stomp, right stomp,rotary: select next/previous parameters in groups of three
Parameter page nr/total parameters
Upper part display: 
up to three times:
  Parameter Name
   parameter value (output of parameterDisplay())
   numerical value as horizontal bar, current positition of p1/p2/p3
enter+p1, p2, p3: set current parameter value
p1, p2, p3 set default parameters
exit: to stompbox mode
*/
#define PPFX_MODE_EDITPARAM 3

/*
left stomp: start, record or overdub
middle stomp: special function(retrigger, to preset mode)
right stomp: stop, delete
enter+p1: direct level
enter+p2: loop level
enter+p3: set global volume
*/
#define PPFX_MODE_LOOPER 4



#ifdef __cplusplus 
extern "C" {
#endif
typedef struct 
{
    uint32_t pressedTimestamp;
    
} ButtonStateType;


typedef void(*uiEnterFct)(void);


const uiEnterFct*  getEnterFunctions();
void registerEnterButtonPressedCallback(void(*cb)(void));
void registerEnterButtonReleasedCallback(void(*cb)(void));
void registerExitButtonPressedCallback(void(*cb)(void));
void registerExitButtonReleasedCallback(void(*cb)(void));
void registerStompswitch1PressedCallback(void(*cb)(void));
void registerStompswitch1ReleasedCallback(void(*cb)(void));
void registerStompswitch2PressedCallback(void(*cb)(void));
void registerStompswitch2ReleasedCallback(void(*cb)(void));
void registerStompswitch3PressedCallback(void(*cb)(void));
void registerStompswitch3ReleasedCallback(void(*cb)(void));
void registerRotaryCallback(void(*cb)(int16_t));
void registerKnob0Callback(void(*cb)(uint16_t));
void registerKnob1Callback(void(*cb)(uint16_t));
void registerKnob2Callback(void(*cb)(uint16_t));
void registerOnUpdateCallback(void(*cb)(int16_t,int16_t,uint8_t));
void registerOnCreateCallback(void(*cb)(void));
void clearCallbackAssignments();

void onEnterPressed(void);
void onEnterReleased(void);
void onExitPressed(void);
void onExitReleased(void);
void onRotaryChange(int16_t delta);
void onKnob0(uint16_t val);
void onKnob1(uint16_t val);
void onKnob2(uint16_t val);
void onStompSwitch1Pressed(void);
void onStompSwitch1Released(void);
void onStompSwitch2Pressed(void);
void onStompSwitch2Released(void);
void onStompSwitch3Pressed(void);
void onStompSwitch3Released(void);
void onUpdate(int16_t avgInput,int16_t avgOutput,uint8_t cpuLoad);
void onCreate(void);
BwImageType * getImageBuffer();

void uiSwitchMode(void);
uint8_t uiStackPush(uint8_t val);
uint8_t uiStackPop();
uint8_t uiStackCurrent();
#ifdef __cplusplus
}
#endif

void piPicoFxUiSetup(void);
void enterLevel0(void);
void enterLevel1(void);
void enterLevel2(void);
void enterLevel3(void);
void enterLevel4(void);
void enterLevel5(void);
void enterLevel6(void);
void enterLevel7(void);
void enterLevel8(void);
void enterLevel9(void);
void enterLevel10(void);
void enterLevel11(void);

void drawBottomPanel(BwImageType* imgBuffer);

void drawMasterVolume(int32_t currentVolume,BwImageType*img);

void drawAbout(BwImageType* imgBuffer);


#endif