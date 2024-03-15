
#include "systemChoice.h"

#ifdef HARDWARE
#ifndef SIMPLE_TIMERTEST
#ifndef SIMPLE_NEOPIXEL


//#include <neopixelDriver.h>
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"

#include "system.h"
#include "core.h"
#include "systemClock.h"
#include "systick.h"
#include "datetimeClock.h"
#include "uart.h"
#include "dma.h"
#include "fmc.h"
#include "qspi.h"
#include "adc.h"
#include "timer.h"
#include "gpio.h"
#include "drivers/oled_display.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "debugLed.h"
#include "consoleHandler.h"
#include "consoleBase.h"
#include "apiHandler.h"
#include "bufferedInputHandler.h"
#include "qspiFlasher.h"
#include "stringFunctions.h"
#include "charDisplay.h"
#include "rotaryEncoder.h"
#include "cliApiTask.h"
#include "sai.h"
#include "i2c.h"
#include "wm8731.h"
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/audiotools.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;


float inputSampleScaled;
volatile float avgOutOld=0,avgInOld=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t cpuLoad;
PiPicoFxUiType piPicoUiController;

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define ADC_LOWPASS 60
#define UI_DMIN 16
uint32_t encoderVal,encoderCntr,encNew;
int16_t encoderDelta;
uint8_t enterSwitchVal;
uint8_t exitSwitchVal;
char displayData[128];
uint16_t adcVal;
volatile uint8_t programsActivated=0;
const uint8_t stompswitch_progs[]={8,7,1};
volatile uint8_t programsToInitialize[3];
FxPresetType presets[3];
volatile uint8_t currentBank=0;
volatile uint8_t currentPreset=0;
// 0: done
// 1: change request
// 2: fade out
// 3: in bypass / change in progress
// 4: fade in
volatile uint8_t programChangeState;

int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint16_t cpuLoadBfr;
uint32_t encoderValOld=0,encoderVal;
uint8_t switchVals[2]={0,0};
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;
uint16_t adcChannel=0;

uint32_t tickStart, tickEnd;
char chrbfr[16];

static volatile uint32_t * audioStatePtr;


/**
 * @brief the main entry point, should never exit
 * 
 * @return int by definition but should never return a value
 */
int main(void)
{


	/*
	 *
	 * Initialize Hardware components
	 * 
	 * */
    setupClock();
	initSystickTimer();
	initDatetimeClock();
	initUart(57600);
    
	initDMA();
    initFmcSdram();
    initQspi();
	initTimer();
	initAdc();
	initI2c(26); // 26 for wm8731, 72 for cs4270
	

	//Initialise Component-specific drivers
	initOledDisplay();
	setupWm8731(SAMPLEDEPTH_24BIT,SAMPLERATE_48KHZ);
	initRotaryEncoder(switchesPins,2);
	encoderVal=getEncoderValue();
    initDebugLed();

    // wait for flashing when button 0 (Enter switch) is pressed during startup 
    // allows flashing the QSPI from a corrupted state
    volatile uint8_t currentSwitchVal = getMomentarySwitchValue(0);
    if ((currentSwitchVal & 0x01)==1)
    {
        while ((task & (1 << TASK_FLASH_QSPI)) == 0);    
        flashingTask();
        task &= ~(1 << TASK_FLASH_QSPI);
    }

    //Initialize Background Services
	//initCliApi();
	initRoundRobinReading(); // internal adc for reading parameters
	context |= (1 << CONTEXT_USB);
	//printf("Microsys v1.1 running on DaisySeed 1.1\r\n");
	
    piPicoFxUiSetup(&piPicoUiController);
	OledClearDisplay();
	for (uint8_t c=0;c<N_FX_PROGRAMS;c++)
	{
		if ((uint32_t)fxPrograms[c]->setup != 0)
		{
			fxPrograms[c]->setup(fxPrograms[c]->data);
		}
	}
	enterLevel0(&piPicoUiController);
    
    
    //enable audio engine last (when fx programs have been set up)
    initSAI();
    enableAudioEngine();
    
    
    audioStatePtr = getAudioStatePtr();

    /* Loop forever */
	for(;;)
	{
		//cliApiTask(task);

        if ((task & (1 << TASK_FLASH_QSPI)) != 0)
        {
            flashingTask();
            task &= ~(1 << TASK_FLASH_QSPI);
        }
        
        
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
            restartAdc();
        }
        
		
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = (int32_t)(avgInOld*128.0f);
            avgOldOutBfr = (int32_t)(avgOutOld*128.0f);
            cpuLoadBfr = cpuLoad >> 1;
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
		
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {
            onEnterPressed(&piPicoUiController);
            clearPressedStickyBit(0);
        }
        if ((switchVals[0] & 2) > 0)
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
                encoderDelta = 1;
            }
            else
            {
                encoderDelta = -1;
            }
           onRotaryChange(encoderDelta,&piPicoUiController);
           clearStickyIncrementDelta();
       }
       
	
	}
}
#endif
#endif
#endif
