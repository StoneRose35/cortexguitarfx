extern "C" {
#include "drivers/dma.h"
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
#include "drivers/display128x64.h"
#include "system.h"
#include "audio/sineplayer.h"
}

#include "pipicofx/FxProgram.hpp"
int16_t* audioBufferPtr;
#ifndef I2S_INPUT
uint16_t* audioBufferInputPtr;
#else
int16_t* audioBufferInputPtr;
#endif
int16_t inputSample, outputSample;

#define AVERAGING_LOWPASS_CUTOFF 10

volatile int16_t avgOut=0,avgIn=0;

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

extern "C" {
__attribute__ ((section (".ramfunc"))) 
void isr_c0_dma_irq0_irq11()
{
	// re-enable dma interrupts
	*NVIC_ISER = (1 << 11);
	return;
}


void isr_c1_dma_irq0_irq11()
{
	if ((*DMA_INTS0 & (1<<4))==(1 << 4)) // channel 4: one line of display data written, handled by core 1
	{
		*DMA_INTS0 = (1<<4);
		DisplayWriteNextLine();
	}
	else 
	{
		spurious_irq_cntr++;
	}
	
	return;
}
}