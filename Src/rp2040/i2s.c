
#include "hardware/regs/addressmap.h"
#include "hardware/regs/pio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/dma.h"
#include "gen/pio0_pio.h"
#include "i2s.h"
#include "adc.h"
#include "hardware/rp2040_registers.h"



static int16_t i2sDoubleBuffer[AUDIO_BUFFER_SIZE*2*2];
#ifdef I2S_INPUT
static int16_t i2sDoubleBufferIn[AUDIO_BUFFER_SIZE*2*2];
#endif
static volatile  uint32_t dbfrPtr; 
static volatile uint32_t dbfrInputPtr;
volatile uint32_t audioState;


void isr_pio1_irq0_irq9()
{
	if((*PIO1_IRQ & (1 << 0)) == (1 << 0))
	{
		startConversion();
		*PIO1_IRQ = (1 << 0);
	}
}


void initI2S()
{
    uint16_t instr_mem_cnt = 0;
	uint16_t first_instr_pos;

	audioState = 0;
 
    first_instr_pos = instr_mem_cnt;
    // disable optional side-set, set wrap top and wrap bottom
	*PIO1_SM0_EXECCTRL = (0 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
	| ( (i2s_duplex_wrap_target + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (i2s_duplex_wrap + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_TOP_LSB);

	//  autopush and autopull
	// shift out to left and in from right 
	*PIO1_SM0_SHIFTCTRL = (0 << PIO_SM0_SHIFTCTRL_PULL_THRESH_LSB) 
                           | (1 << PIO_SM0_SHIFTCTRL_AUTOPULL_LSB) 
                           | (0 << PIO_SM0_SHIFTCTRL_FJOIN_TX_LSB) 
						   | (0 << PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_LSB)

						   | (0 << PIO_SM0_SHIFTCTRL_PUSH_THRESH_LSB)
						   | (1 << PIO_SM0_SHIFTCTRL_AUTOPUSH_LSB)
						   | (0 << PIO_SM0_SHIFTCTRL_FJOIN_RX_LSB)
						   | (0 << PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_LSB)
						   ;

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < i2s_duplex_program.length;c++){
		*(PIO1_INSTR_MEM + instr_mem_cnt++) = (*(i2s_duplex_program.instructions + c) & 0xe000)==0 ?
		 *(i2s_duplex_program.instructions + c) + first_instr_pos : *(i2s_duplex_program.instructions + c);
	}

	// data, bclk, and lrclk must be consecutive pins
	// the set range spans over these 3 pins
	// sideset starts at bclk
	// out is data pin
	// in is data in pin
	*PIO1_SM0_PINCTRL = 
	  (2 << PIO_SM0_PINCTRL_SIDESET_COUNT_LSB)
	| (1 << PIO_SM0_PINCTRL_OUT_COUNT_LSB)
    | (I2S_DATA_PIN << PIO_SM0_PINCTRL_OUT_BASE_LSB)
	| (I2S_BCK_PIN << PIO_SM0_PINCTRL_SIDESET_BASE_LSB)
	| (I2S_DATA_PIN << PIO_SM0_PINCTRL_SET_BASE_LSB)
	| (I2S_DATA_IN_PIN << PIO_SM0_PINCTRL_IN_BASE_LSB)
	| (3 << PIO_SM0_PINCTRL_SET_COUNT_LSB)
    ;

	// switch control of i2s pins to pio1
	*I2S_BCK_PIN_CNTR = 7;
    *I2S_DATA_PIN_CNTR = 7;
    *I2S_WS_PIN_CNTR = 7;
	*I2S_DATA_IN_PIN_CNTR = 7;

	// set clock divider : 120Mhz/78 (.125)
	*PIO1_SM0_CLKDIV = (I2S_CLKDIV_DBL_INT << PIO_SM0_CLKDIV_INT_LSB) | (I2S_CLKDIV_DBL_FRAC << PIO_SM0_CLKDIV_FRAC_LSB);

    // set pin directions to output
    *PIO1_SM0_INSTR = 0xE087;

	// set x,0 side 0 ; put default value of 0 into x for the case the software buffer stalls
	*PIO1_SM0_INSTR = 0xE020;

	// jump to first instruction
	*PIO1_SM0_INSTR = first_instr_pos+i2s_duplex_offset_is2_duplex_start;

	// initialize DMA
	// dac output
	*DMA_CH2_WRITE_ADDR = (uint32_t)PIO1_SM0_TXF;
	dbfrPtr = 0;
	*DMA_CH2_READ_ADDR = dbfrPtr + (uint32_t)i2sDoubleBuffer;
	*DMA_CH2_TRANS_COUNT = AUDIO_BUFFER_SIZE;
	*DMA_CH2_CTRL_TRIG = (8 << DMA_CH2_CTRL_TRIG_TREQ_SEL_LSB) 
						| (1 << DMA_CH2_CTRL_TRIG_INCR_READ_LSB) 
						| (2 << DMA_CH2_CTRL_TRIG_DATA_SIZE_LSB) // always read left and right at once
						| (1 << DMA_CH2_CTRL_TRIG_EN_LSB);

	// adc input
	*DMA_CH3_READ_ADDR = (uint32_t)PIO1_SM0_RXF;
	*DMA_CH3_WRITE_ADDR = dbfrInputPtr + (uint32_t)i2sDoubleBufferIn;
	*DMA_CH3_TRANS_COUNT = AUDIO_BUFFER_SIZE;
	*DMA_CH3_CTRL_TRIG = (12 << DMA_CH2_CTRL_TRIG_TREQ_SEL_LSB) // pio1 DREQ_PIO1_RX0
						| (1 << DMA_CH3_CTRL_TRIG_INCR_WRITE_LSB) 
						| (2 << DMA_CH3_CTRL_TRIG_DATA_SIZE_LSB) // always write left and right at once
						| (1 << DMA_CH3_CTRL_TRIG_EN_LSB);

	*DMA_INTE0 |= (1 << 3) | (1 << 2); // assert dma for channel 3 since the adc channel is always finished halb a bitclock cycle later

	#ifndef I2S_INPUT
	*PIO1_INTE |= (1 << PIO_IRQ0_INTE_SM0_LSB);
	#endif


	// ***********************
	//
	// PIO1 STATE MACHINE 2
	//
	// ***********************
	// setup state machine 2 as master clock
	first_instr_pos = instr_mem_cnt;
	// disable optional side-set,
	*PIO1_SM2_EXECCTRL = (0 << PIO_SM2_EXECCTRL_SIDE_EN_LSB) ;

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < square_oscillator_program.length;c++){
		*(PIO1_INSTR_MEM + instr_mem_cnt++) = (*(square_oscillator_program.instructions + c) & 0xe000)==0 ?
		 *(square_oscillator_program.instructions + c) + first_instr_pos : *(square_oscillator_program.instructions + c);
	}

	// enable fast slew rate and disable schmitt trigger for faster clocking
	*I2S_MCLK_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_SLEWFAST_LSB);
	*I2S_MCLK_PAD_CNTR &= ~(1 << PADS_BANK0_GPIO0_SCHMITT_LSB);
	*I2S_MCLK_PIN_CNTR = 7;

	// set clock divider : 120Mhz/(48000*256)
	*PIO1_SM2_CLKDIV = (I2S_CLKDIV_MCLK_INT << PIO_SM2_CLKDIV_INT_LSB) | (I2S_CLKDIV_MCLK_FRAC << PIO_SM2_CLKDIV_FRAC_LSB);

	*PIO1_SM2_PINCTRL = 
	  (1 << PIO_SM2_PINCTRL_SIDESET_COUNT_LSB)
	| (0 << PIO_SM2_PINCTRL_OUT_COUNT_LSB)
	| (I2S_MCLK_PIN << PIO_SM2_PINCTRL_SIDESET_BASE_LSB)
	| (I2S_MCLK_PIN << PIO_SM2_PINCTRL_SET_BASE_LSB)
	| (1 << PIO_SM2_PINCTRL_SET_COUNT_LSB);

	    // set pin directions to output
    *PIO1_SM2_INSTR = 0xE081;

	// jump to first instruction
	*PIO1_SM2_INSTR = first_instr_pos;

	
	#ifndef I2S_INPUT
    // start PIO 1, state machine 0
	*PIO1_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+0));
	#else
	/*
		*PIO1_CTRL = ((1 << 2) << PIO_CTRL_CLKDIV_RESTART_LSB);// | ((1 << 0) << PIO_CTRL_CLKDIV_RESTART_LSB);
		*PIO1_CTRL = ((1 << 0) << PIO_CTRL_CLKDIV_RESTART_LSB);

		*PIO1_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+0)) | (1 << (PIO_CTRL_SM_ENABLE_LSB+2));
		*PIO1_CTRL |= ((1 << 0) << PIO_CTRL_SM_RESTART_LSB) | ((1 << 2) << PIO_CTRL_SM_RESTART_LSB);
*/
		// start master clock a tad earlier that the bit and lrclock
		*PIO1_CTRL |= ((1 << 2) << PIO_CTRL_CLKDIV_RESTART_LSB) | (1 << (PIO_CTRL_SM_ENABLE_LSB+2)) | ((1 << 2) << PIO_CTRL_SM_RESTART_LSB);
		// __asm__("nop"); 
		*PIO1_CTRL |= ((1 << 0) << PIO_CTRL_CLKDIV_RESTART_LSB) | (1 << (PIO_CTRL_SM_ENABLE_LSB+0)) | ((1 << 0) << PIO_CTRL_SM_RESTART_LSB);
	#endif
	audioState = (1 << AUDIO_STATE_ON);
}


void initI2SSlave()
{
    uint16_t instr_mem_cnt = 0;
	uint16_t first_instr_pos;

	audioState = 0;
 
    first_instr_pos = instr_mem_cnt;
    // set wrap top and wrap bottom, set jump pin
	*PIO1_SM0_EXECCTRL = 
	( (i2s_duplex_slave_wrap_target + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (i2s_duplex_slave_wrap + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_TOP_LSB) | (I2S_WS_PIN << PIO_SM0_EXECCTRL_JMP_PIN_LSB);

	// shift out to left and in from right 
	*PIO1_SM0_SHIFTCTRL = (0 << PIO_SM0_SHIFTCTRL_PULL_THRESH_LSB) 
                           | (0 << PIO_SM0_SHIFTCTRL_AUTOPULL_LSB) 
                           | (0 << PIO_SM0_SHIFTCTRL_FJOIN_TX_LSB) 
						   | (0 << PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_LSB)

						   | (0 << PIO_SM0_SHIFTCTRL_PUSH_THRESH_LSB)
						   | (0 << PIO_SM0_SHIFTCTRL_AUTOPUSH_LSB)
						   | (0 << PIO_SM0_SHIFTCTRL_FJOIN_RX_LSB)
						   | (0 << PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_LSB)
						   ;

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < i2s_duplex_slave_program.length;c++){
		*(PIO1_INSTR_MEM + instr_mem_cnt++) = (*(i2s_duplex_slave_program.instructions + c) & 0xe000)==0 ?
		 *(i2s_duplex_slave_program.instructions + c) + first_instr_pos : *(i2s_duplex_slave_program.instructions + c);
	}

    // 3 consecutive input pins: bclk, lrclk and data_in (from adc)
	// one output pin: dout (to dac)
	*PIO1_SM0_PINCTRL = 
	(1 << PIO_SM0_PINCTRL_OUT_COUNT_LSB)
    | (I2S_DATA_PIN << PIO_SM0_PINCTRL_OUT_BASE_LSB)
	| (I2S_DATA_PIN << PIO_SM0_PINCTRL_SET_BASE_LSB)
	| (I2S_DATA_IN_PIN << PIO_SM0_PINCTRL_IN_BASE_LSB)
	| (1 << PIO_SM0_PINCTRL_SET_COUNT_LSB)
    ;

	// switch control of i2s pins to pio1
	*I2S_BCK_PIN_CNTR = 7;
    *I2S_DATA_PIN_CNTR = 7;
    *I2S_WS_PIN_CNTR = 7;
	*I2S_DATA_IN_PIN_CNTR = 7;

	// set clock divider to 1 (fastest possible)
	*PIO1_SM0_CLKDIV = (1 << PIO_SM0_CLKDIV_INT_LSB);

    // set data_pin direction to output
    *PIO1_SM0_INSTR = 0xE081;

	// jump to first instruction
	*PIO1_SM0_INSTR = first_instr_pos;

	// initialize DMA
	// dac output
	*DMA_CH2_WRITE_ADDR = (uint32_t)PIO1_SM0_TXF;
	dbfrPtr = 0;
	*DMA_CH2_READ_ADDR = dbfrPtr + (uint32_t)i2sDoubleBuffer;
	*DMA_CH2_TRANS_COUNT = AUDIO_BUFFER_SIZE;
	*DMA_CH2_CTRL_TRIG = (8 << DMA_CH2_CTRL_TRIG_TREQ_SEL_LSB) 
						| (1 << DMA_CH2_CTRL_TRIG_INCR_READ_LSB) 
						| (2 << DMA_CH2_CTRL_TRIG_DATA_SIZE_LSB) // always read left and right at once
						| (1 << DMA_CH2_CTRL_TRIG_EN_LSB);

	// adc input
	*DMA_CH3_READ_ADDR = (uint32_t)PIO1_SM0_RXF;
	*DMA_CH3_WRITE_ADDR = dbfrInputPtr + (uint32_t)i2sDoubleBufferIn;
	*DMA_CH3_TRANS_COUNT = AUDIO_BUFFER_SIZE;
	*DMA_CH3_CTRL_TRIG = (12 << DMA_CH2_CTRL_TRIG_TREQ_SEL_LSB) // pio1 DREQ_PIO1_RX0
						| (1 << DMA_CH3_CTRL_TRIG_INCR_WRITE_LSB) 
						| (2 << DMA_CH3_CTRL_TRIG_DATA_SIZE_LSB) // always write left and right at once
						| (1 << DMA_CH3_CTRL_TRIG_EN_LSB);

	*DMA_INTE0 |= (1 << 3) | (1 << 2); // assert dma for channel 3 since the adc channel is always finished halb a bitclock cycle later

}

void toggleAudioBuffer()
{
	dbfrPtr += AUDIO_BUFFER_SIZE*4;
	dbfrPtr &= (AUDIO_BUFFER_SIZE*4*2-1);
	*DMA_CH2_READ_ADDR = dbfrPtr + (uint32_t)i2sDoubleBuffer;
	*DMA_CH2_TRANS_COUNT_TRIG = AUDIO_BUFFER_SIZE; // write to alias 1 to trigger dma on writing transmission count
}

#ifdef I2S_INPUT
void toggleAudioInputBuffer()
{
	dbfrInputPtr += AUDIO_BUFFER_SIZE*4;
	dbfrInputPtr &= (AUDIO_BUFFER_SIZE*4*2-1);
	*DMA_CH3_WRITE_ADDR = dbfrInputPtr + (uint32_t)i2sDoubleBufferIn;
	*DMA_CH3_TRANS_COUNT_TRIG = AUDIO_BUFFER_SIZE; // write to alias 1 to trigger dma on writing transmission count
}
#endif

void enableAudioEngine()
{
	*DMA_CH2_CTRL_TRIG |= (1 << DMA_CH2_CTRL_TRIG_EN_LSB);
	#ifdef I2S_INPUT
	*DMA_CH3_CTRL_TRIG |= (1 << DMA_CH3_CTRL_TRIG_EN_LSB);
	#endif
	audioState = (1 << AUDIO_STATE_ON);
}

void disableAudioEngine()
{
	*DMA_CH2_CTRL_TRIG &= ~(1 << DMA_CH2_CTRL_TRIG_EN_LSB);
	#ifdef I2S_INPUT
	*DMA_CH3_CTRL_TRIG &= ~(1 << DMA_CH3_CTRL_TRIG_EN_LSB);
	#endif
	audioState = 0;
}

int16_t* getEditableAudioBuffer()
{
	int16_t * otherBuffer;
	otherBuffer = (int16_t*)(((dbfrPtr + AUDIO_BUFFER_SIZE*2*2) & (AUDIO_BUFFER_SIZE*2*2*2-1)) + (uint32_t)i2sDoubleBuffer);
	return otherBuffer;
}

#ifdef I2S_INPUT
int16_t* getInputAudioBuffer()
{
	int16_t * otherBuffer;
	otherBuffer = (int16_t*)(((dbfrInputPtr + AUDIO_BUFFER_SIZE*2*2) & (AUDIO_BUFFER_SIZE*2*2*2-1)) + (uint32_t)i2sDoubleBufferIn);
	return otherBuffer;
}
#endif