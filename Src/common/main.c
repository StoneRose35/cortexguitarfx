
#include "systemChoice.h"

#ifdef HARDWARE
#ifndef SIMPLE_TIMERTEST
#ifndef SIMPLE_NEOPIXEL


//#include <neopixelDriver.h>
#include "stm32f446/stm32f446xx.h"
#include "stm32f446/stm32f446_cfg_pins.h"

#include "system.h"
#include "core.h"
#include "systemClock.h"
#include "systick.h"
#include "datetimeClock.h"
#include "uart.h"
#include "dma.h"
#include "pio.h"
#include "adc.h"
#include "timer.h"
#include "gpio.h"
#include "ssd1306_display.h"
#include "debugLed.h"
#include "consoleHandler.h"
#include "consoleBase.h"
#include "apiHandler.h"
#include "bufferedInputHandler.h"
#include "stringFunctions.h"
#include "charDisplay.h"
#include "rotaryEncoder.h"
#include "cliApiTask.h"
#include "i2s.h"
#include "i2c.h"
#include "wm8731.h"
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/fxprogram/fxProgram.h"
#include "pipicofx/pipicofxui.h"

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;


int32_t* audioBufferPtr;
#ifndef I2S_INPUT
uint16_t* audioBufferInputPtr;
#else
int32_t* audioBufferInputPtr;
#endif
float inputSample,inputSampleScaled;
int32_t inputSampleInt;
volatile float avgOut=0,avgOutOld=0,avgIn=0,avgInOld=0;
uint16_t bufferCnt=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t ticStart,ticEnd,cpuLoad;
const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 0.0001f
#define ADC_LOWPASS 2
#define UI_DMIN 1
uint32_t encoderVal,encoderCntr,encNew;
uint8_t enterSwitchVal;
uint8_t exitSwitchVal;
char displayData[128];
uint16_t adcVal;


int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint8_t cpuLoadBfr;
uint8_t switchValsOld[2]={0,0};
uint32_t encoderValOld=0,encoderVal;
uint8_t switchVals[2]={0,0};
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;
uint16_t adcChannel=0;

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
	initTimer();
	initAdc();
	initI2c(26);
	
	/*
	 *
	 * Initialise Component-specific drivers
	 * 
	 * */
	initSsd1306Display();
	setupWm8731(SAMPLEDEPTH_24BIT,SAMPLERATE_48KHZ);
	initI2S();
	initRotaryEncoder(switchesPins,2);
	encoderVal=getEncoderValue();


	/*
     *
     * Initialize Background Services
     *
	 */
	initCliApi();
	initRoundRobinReading(); // internal adc for reading parameters
	context |= (1 << CONTEXT_USB);
	printf("Microsys v1.0 running\r\n");
	piPicoFxUiSetup();
	ssd1306ClearDisplay();
	for (uint8_t c=0;c<N_FX_PROGRAMS;c++)
	{
		if ((uint32_t)fxPrograms[c]->setup != 0)
		{
			fxPrograms[c]->setup(fxPrograms[c]->data);
		}
	}
	drawUi(&piPicoUiController);

	ticEnd=0;
	ticStart=0;


    /* Loop forever */
	for(;;)
	{

		//cliApiTask(task);
		if (((task & (1 << TASK_PROCESS_AUDIO))!= 0) && ((task & (1 << TASK_PROCESS_AUDIO_INPUT))!= 0))
		{
			
			ticStart = getTimeLW();

			audioBufferPtr = getEditableAudioBufferHiRes();
			audioBufferInputPtr = getInputAudioBufferHiRes();


			for (uint32_t c=0;c<AUDIO_BUFFER_SIZE*2;c+=2) // count in frame of 4 bytes or two  24bit samples
			{
				
				// convert raw input to float
				inputSampleInt = (int32_t)((uint32_t)*(audioBufferInputPtr + c) & 0x00FFFFFFL) ;
				inputSample=(float)(inputSampleInt);
				
		

				if (inputSample < 0.0f)
				{
					avgIn = -inputSample;
				}
				else
				{
					avgIn = inputSample;
				}
				avgInOld = AVERAGING_LOWPASS_CUTOFF*avgIn + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgInOld);

				//inputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);


				if (inputSample < 0.0f)
				{
					avgOut = -inputSample;
				}
				else
				{
					avgOut = inputSample;
				}
				avgOutOld = AVERAGING_LOWPASS_CUTOFF*avgOut + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgOutOld);

				inputSampleInt=((int32_t)inputSample);
				*(audioBufferPtr+c) = inputSampleInt;  
				*(audioBufferPtr+c+1) = inputSampleInt;
			}
			task &= ~((1 << TASK_PROCESS_AUDIO) | (1 << TASK_PROCESS_AUDIO_INPUT));
			bufferCnt++;

			ticEnd = getTimeLW();
			if(ticEnd > ticStart)
			{
				cpuLoad = ticEnd-ticStart;
				cpuLoad = cpuLoad*196; // *256*256*F_SAMPLING/AUDIO_BUFFER_SIZE/1000000;
				cpuLoad = cpuLoad >> 8;
			}
			
		}		
		if (bufferCnt == UI_UPDATE_IN_SAMPLE_BUFFERS)
		{
			bufferCnt = 0;
			task |= (1 << TASK_UPDATE_AUDIO_UI);
		}
		
		
        if ((task & (1 << TASK_UPDATE_POTENTIOMETER_VALUES)) == (1 << TASK_UPDATE_POTENTIOMETER_VALUES))
        {
            // call the update function of the chosen program
            adcChannel = getChannel0Value();
            adcChannel0 = adcChannel0 + ((ADC_LOWPASS*(adcChannel - adcChannel0)) >> 8);
            if ((adcChannel0 > adcChannelOld0) && (adcChannel0-adcChannelOld0) > UI_DMIN )
            {
                knob0Callback(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }
            else if ((adcChannel0 < adcChannelOld0) && (adcChannelOld0-adcChannel0) > UI_DMIN )
            {
                knob0Callback(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }

            adcChannel = getChannel1Value();
            adcChannel1 = adcChannel1 + ((ADC_LOWPASS*(adcChannel - adcChannel1)) >> 8);
            if ((adcChannel1 > adcChannelOld1) && (adcChannel1-adcChannelOld1) > UI_DMIN )
            {
                knob1Callback(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }
            else if ((adcChannel1 < adcChannelOld1) && (adcChannelOld1-adcChannel1) > UI_DMIN )
            {
                knob1Callback(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }

            adcChannel = getChannel2Value();
            adcChannel2 = adcChannel2 + ((ADC_LOWPASS*(adcChannel - adcChannel2)) >> 8);
            if ((adcChannel2 > adcChannelOld2) && (adcChannel2-adcChannelOld2) > UI_DMIN )
            {
                knob2Callback(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel2;
            }
            else if ((adcChannel2 < adcChannelOld2) && (adcChannelOld2-adcChannel) > UI_DMIN )
            {
                knob2Callback(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel;
            }
            task &= ~(1 << TASK_UPDATE_POTENTIOMETER_VALUES);
            
        }
        
		
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = (int32_t)avgInOld >> 16;
            avgOldOutBfr = (int32_t)avgOutOld >> 16;
            cpuLoadBfr = (cpuLoad >> 1);
            updateAudioUi(avgOldInBfr,avgOldOutBfr,cpuLoadBfr,&piPicoUiController);
            task &= ~(1 << TASK_UPDATE_AUDIO_UI);
        }
		
        switchVals[0] = getSwitchValue(0);
        if (switchVals[0] == 0 && switchValsOld[0] == 1)
        {
            button1Callback(&piPicoUiController);
        }
        switchValsOld[0] = getSwitchValue(0);

        switchVals[1] = getSwitchValue(1);
        if (switchVals[1] == 0 && switchValsOld[1] == 1)
        {
            button2Callback(&piPicoUiController);
        }
        switchValsOld[1] = getSwitchValue(1);
       encoderVal=getEncoderValue();
       if (encoderValOld > encoderVal + 2 || encoderValOld < encoderVal - 2)
       {
           rotaryCallback(encoderVal,&piPicoUiController);
           encoderValOld=encoderVal;
       }
	   
	}
}
#endif
#endif
#endif
