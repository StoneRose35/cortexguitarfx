#include "globalConfig.h"
#include "system.h"
#include "drivers/dma.h"
#include "drivers/multicore.h"
#include "core1Main.h"
#include "drivers/gpio.h"
#include "drivers/irq.h"
#include "drivers/oled_display.h"
#include "audio/firFilter.h"
#include "audio/audiotools.h"
#include "audio/delay.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "pipicofx/picofxCore.h"
#include "drivers/adc.h"
#include "stringFunctions.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "drivers/stompswitches.h"
#include "drivers/systick.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"


int16_t firstHalfOut;
FirFilterType**core1FirData;
extern volatile uint32_t task;
extern volatile int16_t avgOutOld,avgInOld;
extern volatile uint8_t fxProgramIdx;
extern volatile uint32_t cpuLoad;
extern volatile uint8_t programsActivated;
extern volatile uint8_t programChangeState;
extern volatile uint8_t programsToInitialize[3];
extern const uint8_t stompswitch_progs[];
extern PiPicoFxUiType piPicoUiController;
int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint8_t cpuLoadBfr;
int16_t encoderDelta;
uint8_t switchVals[2]={0,0};
//PiPicoFxUiType uiControllerData;
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;
uint16_t adcChannel=0;
uint8_t stompSwitchState;
FxPresetType preset1, preset2;



static volatile uint32_t * audioStatePtr;
#define UI_DMIN 1
#define ADC_LOWPASS 2

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};

void isr_c1_sio_irq_proc1_irq16() // only fires when a fir computation has to be made
{
    if ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB))!= 0)
    {
        core1FirData = (FirFilterType**)*SIO_FIFO_RD;
        firstHalfOut = processFirstHalf(*core1FirData);
        *SIO_FIFO_WR = firstHalfOut;
    }
    else if (((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_ROE_LSB)) != 0) || ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_WOF_LSB)) != 0))
    {
        *SIO_FIFO_ST = (1 << 2);
    }
}

void core1Main()
{
    audioStatePtr = getAudioStatePtr();

    // initalized the rotary encoder and the switches so that core 1 handler the interrupts of the ui elements
    initRotaryEncoder(switchesPins,2);
    #ifdef EXTENSION_BOARD
    initStompSwitchesInterface();
    #endif
    initRoundRobinReading(); // internal adc for reading parameters

    setAsOuput(CLIPPING_LED_INPUT);
    setAsOuput(CLIPPING_LED_OUTPUT);

    #ifdef EXTENSION_BOARD
    setStompswitchColorRaw(0);
    #endif

    *SIO_FIFO_ST = (1 << 2);
    *SIO_FIFO_WR=0xcafeface; // write sync word for core 0 to wait for core 1
    *NVIC_ISER = (1 << 16) | (1 << 11); // enable interrupt for dma and sio of proc1 
    setInterruptPriority(11,1);
    /*
    preset1.bankNr = 0;
    preset1.bankPos = 1;
    preset1.name[0]=0;
    appendToString(preset1.name,"ShortRev");
    preset1.programNr = 8;
    preset1.parameters[0]=2222;
    preset1.parameters[1]=2568;
    preset1.parameters[2]=0;
    savePreset(&preset1,0);
    waitSysticks(10);
    loadPreset(&preset2,0);
    waitSysticks(10);
    */

    for(;;)
    {
        if ((task & (1 << TASK_UPDATE_POTENTIOMETER_VALUES)) == (1 << TASK_UPDATE_POTENTIOMETER_VALUES))
        {
            // call the update function of the chosen program
            adcChannel = getChannel0Value();
            adcChannel0 = adcChannel0 + ((ADC_LOWPASS*(adcChannel - adcChannel0)) >> 8);
            if ((adcChannel0 > adcChannelOld0) && (adcChannel0-adcChannelOld0) > UI_DMIN )
            {
                onKnob0(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }
            else if ((adcChannel0 < adcChannelOld0) && (adcChannelOld0-adcChannel0) > UI_DMIN )
            {
                onKnob0(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }

            adcChannel = getChannel1Value();
            adcChannel1 = adcChannel1 + ((ADC_LOWPASS*(adcChannel - adcChannel1)) >> 8);
            if ((adcChannel1 > adcChannelOld1) && (adcChannel1-adcChannelOld1) > UI_DMIN )
            {
                onKnob1(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }
            else if ((adcChannel1 < adcChannelOld1) && (adcChannelOld1-adcChannel1) > UI_DMIN )
            {
                onKnob1(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }

            adcChannel = getChannel2Value();
            adcChannel2 = adcChannel2 + ((ADC_LOWPASS*(adcChannel - adcChannel2)) >> 8);
            if ((adcChannel2 > adcChannelOld2) && (adcChannel2-adcChannelOld2) > UI_DMIN )
            {
                onKnob2(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel2;
            }
            else if ((adcChannel2 < adcChannelOld2) && (adcChannelOld2-adcChannel) > UI_DMIN )
            {
                onKnob2(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel;
            }
            task &= ~(1 << TASK_UPDATE_POTENTIOMETER_VALUES);
        }
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = avgInOld >> 8;
            avgOldOutBfr = avgOutOld >> 8;
            cpuLoadBfr = (cpuLoad >> 1);
            onUpdate(avgOldInBfr,avgOldOutBfr,cpuLoadBfr,&piPicoUiController);
            if ((*audioStatePtr & (1 << AUDIO_STATE_INPUT_CLIPPED)) == (1 << AUDIO_STATE_INPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_INPUT,1);
                *audioStatePtr &= ~(1 << AUDIO_STATE_INPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_INPUT,0);
            }
            if ((*audioStatePtr & (1 << AUDIO_STATE_OUTPUT_CLIPPED)) == (1 << AUDIO_STATE_OUTPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_OUTPUT,1);
                *audioStatePtr &= ~(1 << AUDIO_STATE_OUTPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_OUTPUT,0);
            }
            task &= ~(1 << TASK_UPDATE_AUDIO_UI);
        }

        /*
         *
         * UI Switches Callback
         * 
        */
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {
            onEnterPressed(&piPicoUiController);
            clearPressedStickyBit(0);
        }
        if((switchVals[0] & 2) > 0)
        {
            onEnterReleased(&piPicoUiController);
            clearReleasedStickyBit(0);
        }

        switchVals[1] = getSwitchValue(1);
        if ((switchVals[1] & 1) > 0)
        {
            onExitPressed(&piPicoUiController);
            clearPressedStickyBit(1);
        }
        if ((switchVals[1] & 2) > 0)
        {
            onExitReleased(&piPicoUiController);
            clearReleasedStickyBit(1);
        }

       encoderDelta=getStickyIncrementDelta();
       if (encoderDelta != 0)
       {
            if (encoderDelta > 0)
            {
                encoderDelta=1;
            }
            else
            {
                encoderDelta=-1;
            }
           onRotaryChange(encoderDelta,&piPicoUiController);
           clearStickyIncrementDelta();
       }

       /*
        *
        * Stomp Switches Callback
        * 
       */
        #ifdef EXTENSION_BOARD
        stompSwitchState = getStompSwitchState(0);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(0);
            onStompSwitch1Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(0);
            onStompSwitch1Released(&piPicoUiController);
        }
        stompSwitchState = getStompSwitchState(1);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(1);
            onStompSwitch2Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(1);
            onStompSwitch2Released(&piPicoUiController);
        }
        stompSwitchState = getStompSwitchState(2);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(2);
            onStompSwitch3Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(2);
            onStompSwitch3Released(&piPicoUiController);
        }


        if (programChangeState == 3)
        {
            clearDelayLine();
            if (programsToInitialize[0] != 0xFF)
            {
                if (fxPrograms[programsToInitialize[0]]->reset != 0)
                {
                    fxPrograms[programsToInitialize[0]]->reset(fxPrograms[programsToInitialize[0]]->data);
                }
                piPicoUiController.currentProgramIdx = programsToInitialize[0];
                programsToInitialize[0]=0xFF;
                piPicoUiController.currentProgram = fxPrograms[piPicoUiController.currentProgramIdx];
                onCreate(&piPicoUiController);
            }

            programChangeState = 4;
        }
        requestSwitchesUpdate();
        #endif
    }
}