
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
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "multicore.h"
#include "core1Main.h"
#include "audio/fxprogram/fxProgram.h"
#include "pipicofx/pipicofxui.h"

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;


int16_t* audioBufferPtr;
#ifndef I2S_INPUT
uint16_t* audioBufferInputPtr;
#else
int16_t* audioBufferInputPtr;
#endif
int16_t inputSample, inputSampleOther;
volatile int16_t avgOut=0,avgOutOld=0,avgIn=0,avgInOld=0;
uint16_t bufferCnt=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t ticStart,ticEnd,cpuLoad;
const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 10

uint32_t encoderVal,encoderCntr,encNew;
uint8_t enterSwitchVal;
uint8_t exitSwitchVal;
char displayData[128];
/*
void TIM2_IRQHandler()
{
	if ((TIM2->SR & (1 << TIM_SR_UIF_Pos))==(1 << TIM_SR_UIF_Pos))
	{
		GPIOB->ODR ^= (1 << LD1);
		TIM2->SR &= ~(1 << TIM_SR_UIF_Pos);
	}
}
*/

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
	enableFpu();

    setupClock();
	//initUsbPll();
	initSystickTimer();
	initDatetimeClock();
	initUart(57600);
	//initDMA();
	//initTimer();
	//initAdc();
	//initI2c(50);
	
	// use time 2 along with the debug led to check is sysclock is correct
	/*
	RCC->APB1ENR |= (1 << RCC_APB1ENR_TIM2EN_Pos);
	RCC->AHB1ENR |= (1 << RCC_AHB1ENR_GPIOBEN_Pos);
	TIM2->DIER |= (1 << TIM_DIER_UIE_Pos);
	TIM2->PSC=45000-1;
	TIM2->ARR=1000;
	TIM2->CR1=(1 << TIM_CR1_CEN_Pos);
	GPIOB->MODER |= (1 << LD1*2);
	// no pullups or pulldowns
	GPIOB->PUPDR &= ~((3 << LD1*2));

	// enable led1
	GPIOB->BSRR |= ((1 << LD1));
	__NVIC_EnableIRQ(TIM2_IRQn);
	*/

	/*
	 *
	 * Initialise Component-specific drivers
	 * 
	 * */
	//initSsd1306Display();
	//initI2S();
	//initDebugLed();
	initRotaryEncoder(switchesPins,2);


	/*
     *
     * Initialize Background Services
     *
	 */
	initCliApi();
	//initRoundRobinReading(); // internal adc for reading parameters
	context |= (1 << CONTEXT_USB);
	printf("Microsys v1.0 running\r\n");
	//piPicoFxUiSetup();
	//ssd1306ClearDisplay();
	//for (uint8_t c=0;c<N_FX_PROGRAMS;c++)
	//{
	//	if ((uint32_t)fxPrograms[c]->setup != 0)
	//	{
	//		fxPrograms[c]->setup(fxPrograms[c]->data);
	//	}
	//}
	//drawUi(&piPicoUiController);

	ticEnd=0;
	ticStart=0;


    /* Loop forever */
	for(;;)
	{

		cliApiTask(task);
		if (((task & (1 << TASK_PROCESS_AUDIO))!= 0) && ((task & (1 << TASK_PROCESS_AUDIO_INPUT))!= 0))
		{
			/*
			ticStart = getTimeLW();

			audioBufferPtr = getEditableAudioBuffer();
			#ifndef I2S_INPUT
			audioBufferInputPtr = getReadableAudioBuffer();
			#else
			audioBufferInputPtr = getInputAudioBuffer();
			#endif

			for (uint8_t c=0;c<AUDIO_BUFFER_SIZE;c++) // count in frame of 4 bytes or two  16bit samples
			{
				// convert raw input to signed 16 bit
				#ifndef I2S_INPUT
				inputSample = (*(audioBufferInputPtr + c) << 4) - 0x7FFF;
				#else
				inputSample=*(audioBufferInputPtr + c*2);
			
				#endif
				//inputSample = getNextSineValue();

				if (inputSample < 0)
				{
					avgIn = -inputSample;
				}
				else
				{
					avgIn = inputSample;
				}
				avgInOld = ((AVERAGING_LOWPASS_CUTOFF*avgIn) >> 15) + (((32767-AVERAGING_LOWPASS_CUTOFF)*avgInOld) >> 15);

				inputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);//fxPrograms[fxProgramIdx]->processSample(inputSample,fxPrograms[fxProgramIdx]->data);


				if (inputSample < 0)
				{
					avgOut = -inputSample;
				}
				else
				{
					avgOut = inputSample;
				}
				avgOutOld = ((AVERAGING_LOWPASS_CUTOFF*avgOut) >> 15) + (((32767-AVERAGING_LOWPASS_CUTOFF)*avgOutOld) >> 15);

				*((uint32_t*)audioBufferPtr+c) = ((uint16_t)inputSample << 16) | (0xFFFF & (uint16_t)inputSample); 

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
			*/
		}
		if (bufferCnt == UI_UPDATE_IN_SAMPLE_BUFFERS)
		{
			bufferCnt = 0;
			task |= (1 << TASK_UPDATE_AUDIO_UI);
		}
		encNew=getEncoderValue();
		if (encNew < encoderVal-2)
		{
			printf("new encoder value ");
			encoderVal = encNew;
			encoderCntr--;
			UInt32ToChar(encoderCntr,displayData);
			printf(displayData);
			printf("\r\n");
		}
		else if (getEncoderValue() > encoderVal+2)
		{
			printf("new encoder value ");
			encoderVal = encNew;
			encoderCntr++;
			UInt32ToChar(encoderCntr,displayData);
			printf(displayData);
			printf("\r\n");
		}
		if (getSwitchValue(0) != enterSwitchVal)
		{
			printf("new enter switch value ");
			enterSwitchVal = getSwitchValue(0);
			UInt8ToChar(enterSwitchVal,displayData);
			printf(displayData);
			printf("\r\n");
		}
		if (getSwitchValue(1) != exitSwitchVal)
		{
			printf("new exit switch value ");
			exitSwitchVal = getSwitchValue(1);
			UInt8ToChar(exitSwitchVal,displayData);
			printf(displayData);
			printf("\r\n");
		}
	}
}
#endif
#endif
#endif
