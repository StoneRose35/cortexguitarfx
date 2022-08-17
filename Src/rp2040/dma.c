
#include "dma.h"
#include "neopixelDriver.h"
#include "i2s.h"
#include "adc.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/m0plus.h"
#include "hardware/regs/dma.h"
#include "hardware/rp2040_registers.h"

extern volatile uint8_t sendState;
extern volatile uint32_t task;
extern volatile uint32_t audioState;

void initDMA()
{
	// enable the dma block
	*RESETS |= (1 << RESETS_RESET_DMA_LSB);
    *RESETS &= ~(1 << RESETS_RESET_DMA_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_DMA_LSB)) == 0);

	// enable the dma interrupt by default
	*NVIC_ISER = (1 << 11);
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
	else if ((*DMA_INTS0 & (1<<2))==(1 << 2)) // from channel 2: toggle i2s buffer pointer
	{
		*DMA_INTS0 = (1<<2);
		if ((task & (1 << TASK_PROCESS_AUDIO)) == 0)
		{
			audioState &= ~(1 << AUDIO_STATE_BUFFER_UNDERRUN);
		}
		else
		{
			audioState  |= (1 << AUDIO_STATE_BUFFER_UNDERRUN);
		}
		toggleAudioBuffer();
		task |= (1 << TASK_PROCESS_AUDIO);
	}
	else if ((*DMA_INTS0 & (1<<3))==(1 << 3)) // from channel 3: toogle audio input buffer
	{
		*DMA_INTS0 = (1<<3);
		if ((task & (1 << TASK_PROCESS_AUDIO_INPUT)) == 0)
		{
			audioState &= ~(1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
		}
		else
		{
			audioState  |= (1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
		}
		toggleAudioInputBuffer();
		task |= (1 << TASK_PROCESS_AUDIO_INPUT);
	}
	return;
}