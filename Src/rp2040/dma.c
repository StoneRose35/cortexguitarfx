
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
extern volatile int16_t avgOutOld;
extern volatile int16_t avgInOld;
extern uint32_t cpuLoad;
extern volatile uint8_t programChangeState;
extern PiPicoFxUiType piPicoUiController;
static volatile uint32_t * audioStatePtr;
int16_t fadeCounter;
volatile uint32_t spurious_irq_cntr=0;

extern volatile int16_t *currentSamplePointer;
extern volatile uint8_t sampleSelectorVal;
extern volatile uint32_t currentSamplePosition;
extern int16_t **samplePointers;
extern uint32_t sampleLengths[];

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
    if ((*DMA_INTS0 & (1<<1))==(1 << 1) ) // from channel 1: usb uart transmission done, handled by core0
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

			if (((uint32_t)currentSamplePointer)!=0xFFFFFFFF)
			{
				outputSample = *(currentSamplePointer +currentSamplePosition++);
				if (currentSamplePosition >= sampleLengths[sampleSelectorVal])
				{
					currentSamplePointer = (int16_t*)0xFFFFFFFF;
					currentSamplePosition=0;
				}
			}
			else
			{
				outputSample = 0;
			}


			if (outputSample < 0)
			{
				avgOut = -outputSample;
			}
			else
			{
				avgOut = outputSample;
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