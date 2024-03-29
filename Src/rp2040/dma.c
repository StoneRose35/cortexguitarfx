
#include "drivers/dma.h"
#include "drivers/neopixelDriver.h"
#include "drivers/i2s.h"
#include "drivers/adc.h"
#include "drivers/gpio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/regs/dma.h"
#include "hardware/regs/pio.h"
#include "hardware/rp2040_registers.h"
#include "drivers/timer.h"
#include "pipicofx/pipicofxui.h"
#include "audio/audiotools.h"
#include "drivers/oled_display.h"

int16_t* audioBufferPtr;
#ifndef I2S_INPUT
uint16_t* audioBufferInputPtr;
#else
int16_t* audioBufferInputPtr;
#endif
int16_t inputSample, outputSample;

#define AVERAGING_LOWPASS_CUTOFF 10

extern volatile uint8_t sendState;
extern volatile uint32_t task;
extern uint32_t ticStart,ticEnd;
extern uint16_t bufferCnt;
volatile int16_t avgOut=0,avgIn=0;
extern int16_t avgOutOld,avgInOld;
extern uint32_t cpuLoad;
extern volatile uint8_t programChangeState;
extern PiPicoFxUiType piPicoUiController;
static volatile uint32_t * audioStatePtr;
int16_t fadeCounter;
volatile uint32_t spurious_irq_cntr=0;

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
void isr_c0_dma_irq0_irq11()
{
	/*
	if ((*DMA_INTS0 & (1<<0))==(1 << 0)) // if from channel 0: neopixel  frame timer
	{
		// clear interrupt
		*DMA_INTS0 = (1<<0);

		// disable dma channel 0
		*DMA_CH0_CTRL_TRIG &= ~(1 << 0);

		sendState = SEND_STATE_SENT;
	}
	else */if ((*DMA_INTS0 & (1<<1))==(1 << 1) ) // from channel 1: usb uart transmission done, handled by core0
	{
		*DMA_INTS0 = (1<<1);
		*DMA_CH1_CTRL_TRIG &= ~(1 << DMA_CH1_CTRL_TRIG_EN_LSB); // disable dma channel 1
		task |= (1 << TASK_USB_CONSOLE_TX);
	}
	else if ((*DMA_INTS0 & (1<<3))==(1 << 3) ) // from channel 3: toogle audio input buffer, handled by core0
	{
		*DMA_INTS0 = (1<<3);
		// disable other dma interrupts when processing audio
		*NVIC_ICER = (1 << 11);
		toggleAudioBuffer();	
		toggleAudioInputBuffer();



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
			inputSample=*(audioBufferInputPtr + c*2 + 1) + *(audioBufferInputPtr + c*2);
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

			if (programChangeState != 3) // processing
			{
				outputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);
			}
			else
			{
				outputSample = 0;
			}
			if (programChangeState == 2)// fadeout
			{
				outputSample = ((32767 - fadeCounter)*inputSample >> 15) + ((fadeCounter*outputSample) >> 15);
				fadeCounter -= 256;
				if (fadeCounter < 0)
				{
					fadeCounter = 0;
					programChangeState=3;
				}
			}
			else if (programChangeState==4) // fadein
			{
				outputSample = ((32767 - fadeCounter)*inputSample >> 15) + ((fadeCounter*outputSample) >> 15);
				fadeCounter += 256;
				if (fadeCounter < 0) // overrun
				{
					programChangeState = 0;
				}
			}
			if (programChangeState == 1)
			{
				fadeCounter = 32767;
				programChangeState = 2;
			}

			if (inputSample < 0)
			{
				avgOut = -inputSample;
			}
			else
			{
				avgOut = inputSample;
			}
			avgOutOld = ((AVERAGING_LOWPASS_CUTOFF*avgOut) >> 15) + (((32767-AVERAGING_LOWPASS_CUTOFF)*avgOutOld) >> 15);

			*((uint32_t*)audioBufferPtr+c) = ((uint16_t)outputSample << 16) | (0xFFFF & (uint16_t)outputSample); 

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
		// re-enable dma interrupts
		*NVIC_ISER = (1 << 11);
	}
	return;
}



void isr_c1_dma_irq0_irq11()
{
	if ((*DMA_INTS0 & (1<<4))==(1 << 4)) // channel 4: one line of display data written, handled by core 1
	{
		*DMA_INTS0 = (1<<4);
		OledWriteNextLine();
	}
	else 
	{
		spurious_irq_cntr++;
	}
	
	return;
}