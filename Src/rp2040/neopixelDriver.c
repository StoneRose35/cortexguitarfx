/*
 * led_timer.c
 *
 *  Created on: Jul 27, 2021
 *      Author: philipp
 */
#ifdef RP2040_FEATHER
#ifndef SIMPLE_TIMERTEST
#ifndef SIMPLE_NEOPIXEL

#include "neopixelDriver.h"
#include "system.h"
#ifndef _PIO_0_H_
#define PIO_0_H_
#include "gen/pio0_pio.h"
#endif
#include "hardware/regs/pio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/dma.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"


volatile uint8_t sendState=SEND_STATE_INITIAL;

// framerate timer irq function
// should update sendState every frame to notify the mainloop that a new  color info should be sent
// and should also update sendState once clocking in the colors is done a.k.a. when decompressArray can be called earliest
void isr_pio0_irq0_irq7()
{
	if((*PIO_IRQ & (1 << 1)) == (1 << 1)) // got irq 0, a frame has passed
	{
		if (sendState != SEND_STATE_DATA_READY)
		{
			sendState = SEND_STATE_BUFFER_UNDERRUN;
		}
		else
		{
			sendState = SEND_STATE_RTS;
		}
		*PIO_IRQ = (1 << 1);
	}
}



// converts the structured color data for all lamp (frame) into a streamable array handleable by the driver
// should modify the array pointed to by rawdata_ptr
void decompressRgbArray(RGBStream * frame,uint8_t length)
{
	uint8_t cnt = 0;
	uint16_t rdata_cnt = 0;
	*(rawdata_ptr + rdata_cnt) = 0;
	for(cnt=0;cnt<length;cnt++)
	{
		// refer to specific hardware implementation to check which color comes first
		*(rawdata_ptr + rdata_cnt++) = 0;
		*(rawdata_ptr + rdata_cnt++) = (*(frame+cnt)).rgb.b;
		*(rawdata_ptr + rdata_cnt++) = (*(frame+cnt)).rgb.r;
		*(rawdata_ptr + rdata_cnt++) = (*(frame+cnt)).rgb.g;
	}
	sendState = SEND_STATE_DATA_READY;
}

/*
 * Initiates the framerate timer, rawdata transfer 
   mechanism (usually dma) and the hardware driver necessary for operating the neopixel array
 * should be called once after startup
 */
void initNeopixels()
{

	// Swich on Neopixel Power in RP2040 itsybitsy
	#ifdef ITSYBITSY
	*GPIO_OE &= ~(1 << NEOPIXEL_POWER_PIN);
	*GPIO_OUT &= ~(1 << NEOPIXEL_POWER_PIN);
    *NEOPIXEL_POWER_PIN_CNTR =  5; // function 5 (SIO)
	*GPIO_OE |= (1 << NEOPIXEL_POWER_PIN);
	*(GPIO_OUT + 1) = (1 << NEOPIXEL_POWER_PIN);
	#endif
    
	// read from the raw data array in memory
	*DMA_CH0_READ_ADDR = (uint32_t)rawdata_ptr;
	//place data into the TX fifo of PIO0's SM0
	*DMA_CH0_WRITE_ADDR = PIO0_BASE+PIO_TXF0_OFFSET;
	//increase read address at each transfer, select DREQ0 (DREQ_PIO0_TX0) as data sed request
	// choose word-sized transfers
	*DMA_CH0_CTRL_TRIG |= (1 << DMA_CH0_CTRL_TRIG_INCR_READ_LSB) | (0 << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB) | (2 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB);
	//define the number of data sent
	*DMA_CH0_TRANS_COUNT = N_LAMPS;
	//generate interrupt upon completion of the data transfer which happens on channel 0
	*DMA_INTE0 |= (1 << 0);
	
	

	// 
	// PIO Setup
	//
	uint16_t instr_mem_cnt = 0;
	uint16_t first_instr_pos;

	// ***********************************************************
	// configure state machine 0 which drives the neopixels
	// ***********************************************************

	// switch the neopixel pin to be controlled by the pio0
	*NEOPIXEL_PIN_CNTR =  6; 

    first_instr_pos = instr_mem_cnt;
    // enable side-set, set wrap top and wrap bottom
	*PIO_SM0_EXECCTRL = (0 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
	| ( (ws2812_wrap_target + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (ws2812_wrap + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_TOP_LSB);

	//  do pull after 24 bits of have been shifted out, enable autopull
	// shift out left since msb should come first
	*PIO_SM0_SHIFTCTRL |= (24 << PIO_SM1_SHIFTCTRL_PULL_THRESH_LSB) |(1 << PIO_SM1_SHIFTCTRL_AUTOPULL_LSB);
	*PIO_SM0_SHIFTCTRL &= ~(1 << PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_LSB);

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < ws2812_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(ws2812_program.instructions + c) & 0xe000)==0 ?
		 *(ws2812_program.instructions + c) + first_instr_pos : *(ws2812_program.instructions + c);
	}

    // set one sideset pin, base to the neopixel output pin
	// also put the set boundary to the neopixel pin to allow
	// setting the pin as output
	*PIO_SM0_PINCTRL = 
      (1 << PIO_SM0_PINCTRL_SIDESET_COUNT_LSB) 
	| (NEOPIXEL_PIN << PIO_SM0_PINCTRL_SIDESET_BASE_LSB) 
    | ( 1 << PIO_SM0_PINCTRL_SET_COUNT_LSB)
	| ( NEOPIXEL_PIN << PIO_SM0_PINCTRL_SET_BASE_LSB) 
    ;

	// set counter, based on f_sys/(800kHz*10)
	*PIO_SM0_CLKDIV = NP_CLKDIV << PIO_SM0_CLKDIV_INT_LSB;

     // set pindirs, 1
    *PIO_SM0_INSTR = 0xe081;

	// jump to first instruction
	*PIO_SM0_INSTR = first_instr_pos;

    // start PIO 0, state machine 0
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+0));

	// ***********************************************************
	// configure state machine 1 which serves as a frame timer
	// ***********************************************************
	// disable side-set, set wrap top and wrap bottom
	first_instr_pos = instr_mem_cnt;
	*PIO_SM1_EXECCTRL = (0 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
	| ( (frametimer_wrap_target + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (frametimer_wrap + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_TOP_LSB);

	//  do pull after 32 bits of have been shifted out, disable autopull
	*PIO_SM1_SHIFTCTRL |= (0 << PIO_SM1_SHIFTCTRL_PULL_THRESH_LSB) |(0 << PIO_SM1_SHIFTCTRL_AUTOPULL_LSB);

	// fill in instructions
	first_instr_pos = instr_mem_cnt;
	for(uint8_t c=0;c < frametimer_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(frametimer_program.instructions + c) & 0xe000)==0 ?
		 *(frametimer_program.instructions + c) + first_instr_pos : *(frametimer_program.instructions + c);
	}

	// emplicitely jump to first instruction
	*PIO_SM1_INSTR = first_instr_pos;

	//enable interrupt from pio0 sm1
	*PIO_INTE |= (1 << PIO_IRQ0_INTS_SM1_LSB);

	// enable interrupts 7 and 11
	*NVIC_ISER = (1 << 7) | (1 << 11);

    // write the appropriate wait value to the transmit fifo
	*PIO_SM1_TXF = (uint32_t)PIO_SM1_CNT;

	// start PIO 0, state machine 1
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+1));



}

/* non-blocking function which initiates a data transfer to the neopixel array
 * be aware that rawdata should not be modified as long as the data transfer is ongoing
 * */
void sendToLed()
{
	sendState = SEND_STATE_SENDING;
	// reset address
	*DMA_CH0_READ_ADDR = (uint32_t)rawdata_ptr;

	// set the number of transfers
	*DMA_CH0_TRANS_COUNT = N_LAMPS;

	// enable dma
	*DMA_CH0_CTRL_TRIG |= (1 << 0);
}

/**
 * @brief switch the neopixel driver on or off, when off no interrupts due to the frame timer should happen anyore
 * 
 * @param flag 0: off, 1: on
 */
void setEngineState(uint8_t flag)
{
	if(flag== 1)
	{
		// start PIO 0, state machine 1 (the frame timer)
		*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+1));
	}
	else if (flag == 0)
	{
		// stop PIO 0, state machine 1 (the frame timer)
		*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+1));
	}
}

uint8_t getEngineState()
{
	uint8_t retval = ((*PIO_CTRL & (1 << (PIO_CTRL_SM_ENABLE_LSB+1))) == (1 << (PIO_CTRL_SM_ENABLE_LSB+1))) ? 1 : 0;
	return retval;
}

uint8_t getSendState()
{
	return sendState;
}

void setSendState(uint8_t s)
{
	sendState = s;
}

#endif
#endif
#endif
