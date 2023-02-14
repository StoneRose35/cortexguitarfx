
#include "dma.h"
#include "neopixelDriver.h"
#include "i2s.h"
#include "adc.h"
#include "gpio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/regs/dma.h"
#include "hardware/regs/pio.h"
#include "hardware/rp2040_registers.h"
#include "timer.h"
#include "pipicofx/pipicofxui.h"
#include "audio/audiotools.h"

int16_t* audioBufferPtr;
#ifndef I2S_INPUT
uint16_t* audioBufferInputPtr;
#else
int16_t* audioBufferInputPtr;
#endif
int16_t inputSample, inputSampleOther;

#define AVERAGING_LOWPASS_CUTOFF 10

extern volatile uint8_t sendState;
extern volatile uint32_t task;
extern uint32_t ticStart,ticEnd;
extern uint16_t bufferCnt;
volatile int16_t avgOut=0,avgIn=0;
extern int16_t avgOutOld,avgInOld;
extern uint32_t cpuLoad;
extern PiPicoFxUiType piPicoUiController;
static volatile uint32_t * audioStatePtr;

void initDMA()
{
	// enable the dma block
	*RESETS |= (1 << RESETS_RESET_DMA_LSB);
    *RESETS &= ~(1 << RESETS_RESET_DMA_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_DMA_LSB)) == 0);

	// enable the dma interrupt by default
	*NVIC_ISER = (1 << 11);
	audioStatePtr=getAudioStatePtr();
}


/**
 * @brief dma interrupt handler
 * on channel 0 an interrupt is asserted when the neopixel data has been fully clocked out
 * on channel 1 an interrupt is asserted when data has been sent over the usb uart
 */
void isr_dma_irq0_irq11()
{
	if ((*DMA_INTS0 & (1<<0))==(1 << 0)) // if from channel 0: neopixel  frame timer
	{
		// clear interrupt
		*DMA_INTS0 = (1<<0);

		// disable dma channel 0
		*DMA_CH0_CTRL_TRIG &= ~(1 << 0);

		sendState = SEND_STATE_SENT;
	}
	else if ((*DMA_INTS0 & (1<<1))==(1 << 1)) // from channel 1: usb uart transmission done
	{
		*DMA_INTS0 = (1<<1);
		*DMA_CH1_CTRL_TRIG &= ~(1 << DMA_CH1_CTRL_TRIG_EN_LSB); // disable dma channel 1
		task |= (1 << TASK_USB_CONSOLE_TX);
	}
	else if ((*DMA_INTS0 & (1<<3))==(1 << 3)) // from channel 3: toogle audio input buffer
	{
		toggleAudioBuffer();	
		toggleAudioInputBuffer();
		*DMA_INTS0 = (1<<3);


		if ((task & (1 << TASK_PROCESS_AUDIO_INPUT)) == 0)
		{
			*getAudioStatePtr() &= ~(1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
		}
		else
		{
			*getAudioStatePtr()  |= (1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
		}

		ticStart = getTimeLW();
		task |= (1 << TASK_PROCESS_AUDIO_INPUT);

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
			inputSample=*(audioBufferInputPtr + c*2 + 1);
			#endif

			if (inputSample < 0)
			{
				avgIn = -inputSample;
			}
			else
			{
				avgIn = inputSample;
			}
			if ((avgIn & (0x7FFF - 0x3)) == (0x7FFF - 0x3)) // give a little margin to prematurely indicate clipping
			{
				*audioStatePtr |= (1 << AUDIO_STATE_INPUT_CLIPPED);
			}
			avgInOld = ((AVERAGING_LOWPASS_CUTOFF*avgIn) >> 15) + (((32767-AVERAGING_LOWPASS_CUTOFF)*avgInOld) >> 15);

			inputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);


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
		task &= ~((1 << TASK_PROCESS_AUDIO_INPUT)); 
		bufferCnt++;

		ticEnd = getTimeLW();
		if(ticEnd > ticStart)
		{
			cpuLoad = ticEnd-ticStart;
			cpuLoad = cpuLoad*196; //*256*256*F_SAMPLING/AUDIO_BUFFER_SIZE/1000000;
			cpuLoad = cpuLoad >> 8;
		}
	}
	return;
}