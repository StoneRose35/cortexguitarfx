#include <stdint.h>
#ifndef _PIO_0_H_
#define PIO_0_H_
#include "gen/pio0_pio.h"
#endif
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/pio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
#include "ds18b20.h"
#include "systick.h"


static volatile uint32_t tempreadTicks = 0;
static volatile uint8_t tempReadState = 0;


uint8_t getTempReadState()
{
	return tempReadState;
}

uint8_t initDs18b20()
{
    uint16_t instr_mem_cnt = DS18B20_INSTR_MEM_OFFSET;
	uint16_t first_instr_pos;

	// enable pullup on the ds18b20 pin
    *DS18B20_PAD_CNTR &= ~(1 << PADS_BANK0_GPIO0_PDE_LSB);
    *DS18B20_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_PUE_LSB);    

    first_instr_pos = instr_mem_cnt;
    // disable optional side-set, set wrap top and wrap bottom
	*PIO_SM2_EXECCTRL = (0 << PIO_SM2_EXECCTRL_SIDE_EN_LSB) 
	| ( (ds18b20read_wrap_target + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (ds18b20read_wrap + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_TOP_LSB);

	//  push after 8 bits have been read, enable autopush
	*PIO_SM2_SHIFTCTRL |= (8 << PIO_SM2_SHIFTCTRL_PUSH_THRESH_LSB) |(1 << PIO_SM2_SHIFTCTRL_AUTOPUSH_LSB);

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < ds18b20read_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(ds18b20read_program.instructions + c) & 0xe000)==0 ?
		 *(ds18b20read_program.instructions + c) + first_instr_pos : *(ds18b20read_program.instructions + c);
	}


	// define DS18B20 as a set pin and as the lowest input pin
	// define one set pin
	*PIO_SM2_PINCTRL = 
      (1 << PIO_SM2_PINCTRL_SET_COUNT_LSB) 
	| (1 << PIO_SM2_PINCTRL_SIDESET_COUNT_LSB)
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SET_BASE_LSB) 
    | (DS18B20_PIN << PIO_SM2_PINCTRL_IN_BASE_LSB)
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SIDESET_BASE_LSB)
    ;

	// switch control of DS18B20 to Pio
	*DS18B20_PIN_CNTR = 6;

	// set clock divider : 120*15
	*PIO_SM2_CLKDIV = DS18B20_CLKDIV << PIO_SM2_CLKDIV_INT_LSB;

	// jump to first instruction
	*PIO_SM2_INSTR = first_instr_pos;

    // start PIO 0, state machine 2
	//*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	return 0;
}



uint8_t resetDs18b20()
{
	uint32_t rdata;
    uint16_t instr_mem_cnt = DS18B20_INSTR_MEM_OFFSET;
	uint16_t first_instr_pos;

	//disable pio0, sm2
	*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	//restart sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_RESTART_LSB+2));

	//load reset program to sm2
	first_instr_pos = instr_mem_cnt;
	for(uint8_t c=0;c < ds18b20reset_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(ds18b20reset_program.instructions + c) & 0xe000)==0 ?
		 *(ds18b20reset_program.instructions + c) + first_instr_pos : *(ds18b20reset_program.instructions + c);
	}

	// disable autopull and autopush
	*PIO_SM2_EXECCTRL = (0 << PIO_SM2_EXECCTRL_SIDE_EN_LSB); 
	*PIO_SM2_SHIFTCTRL = PIO_SM2_SHIFTCTRL_RESET;

	// set clock divider
	*PIO_SM2_CLKDIV = DS18B20_RESET_CLKDIV << PIO_SM2_CLKDIV_INT_LSB;

	*PIO_SM2_PINCTRL = 
      (1 << PIO_SM2_PINCTRL_SET_COUNT_LSB) 
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SET_BASE_LSB) 
    | (DS18B20_PIN << PIO_SM2_PINCTRL_IN_BASE_LSB)
    ;

	// jump to first instruction
	*PIO_SM2_INSTR = first_instr_pos;

	// enable pio0, sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	// wait until irq 4 is asserted
	while ((*PIO_IRQ & (1 << 4)) == 0);

	// disable sm2, deassert irq4
	*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+2));
	*PIO_IRQ |= (1 << 4);

	// read rx fifo
	rdata = *PIO_SM2_RXF;
	
	// return 0 if at least 1 zero has been sampled
	if( (rdata >> 24) < 0xFF)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

uint8_t writeDs18b20(uint8_t cmd)
{
    uint16_t instr_mem_cnt = DS18B20_INSTR_MEM_OFFSET;
	uint16_t first_instr_pos;

	//disable pio0, sm2
	*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	//restart sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_RESTART_LSB+2));

	// drain tx fifo
	while ((*PIO_FSTAT & (4 << PIO_FSTAT_TXEMPTY_LSB)) == 0)
	{
		*PIO_SM2_INSTR = 0x88; //pull
	}

	//load write program to sm2
	first_instr_pos = instr_mem_cnt;
	for(uint8_t c=0;c < ds18b20write_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(ds18b20write_program.instructions + c) & 0xe000)==0 ?
		 *(ds18b20write_program.instructions + c) + first_instr_pos : *(ds18b20write_program.instructions + c);
	}

	// enable autopull, set wrap boundaries
	*PIO_SM2_EXECCTRL = (0 << PIO_SM2_EXECCTRL_SIDE_EN_LSB) 
	| ( (ds18b20write_wrap_target + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (ds18b20write_wrap + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_TOP_LSB);
	*PIO_SM2_SHIFTCTRL = PIO_SM2_SHIFTCTRL_RESET;
	*PIO_SM2_SHIFTCTRL |= (1 << PIO_SM2_SHIFTCTRL_AUTOPULL_LSB) | (8 << PIO_SM2_SHIFTCTRL_PULL_THRESH_LSB);
	//*PIO_SM2_SHIFTCTRL &= ~(1 << PIO_SM2_SHIFTCTRL_OUT_SHIFTDIR_LSB);

	// set clock divider
	*PIO_SM2_CLKDIV = DS18B20_CLKDIV << PIO_SM2_CLKDIV_INT_LSB;

	*PIO_SM2_PINCTRL = 
	(1 << PIO_SM2_PINCTRL_SET_COUNT_LSB) 
	| (1 << PIO_SM2_PINCTRL_SIDESET_COUNT_LSB)
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SET_BASE_LSB) 
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SIDESET_BASE_LSB)
    ;

    // set as output (set pindirs, 1 side 1)
	*PIO_SM2_INSTR = 0xf081;

	// jump to first instruction, side 1
	*PIO_SM2_INSTR = first_instr_pos | 0x1000;

	// put command to write into the tx fifo
	*PIO_SM2_TXF = cmd; // << 24;

	// clear the tx stall flag
	*PIO_FDEBUG = 4 << PIO_FDEBUG_TXSTALL_LSB;

	// enable pio0, sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	// wait until tx stall flag is on
	while((*PIO_FDEBUG & (4 <<PIO_FDEBUG_TXSTALL_LSB)) == 0);

	return 0;
}


uint8_t readDs18b20()
{
	uint32_t rdata;
    uint16_t instr_mem_cnt = DS18B20_INSTR_MEM_OFFSET;
	uint16_t first_instr_pos;

	// deassert irq 5
	//*PIO_IRQ |= (1 << 5);

	//disable pio0, sm2
	*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	//restart sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_RESTART_LSB+2));

	// drain rx fifo
	while ((*PIO_FSTAT & (4 << PIO_FSTAT_RXEMPTY_LSB)) == 0)
	{
		rdata=*PIO_SM2_RXF;
	}

	//load read program to sm2
	first_instr_pos = instr_mem_cnt;
	for(uint8_t c=0;c < ds18b20reset_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(ds18b20read_program.instructions + c) & 0xe000)==0 ?
		 *(ds18b20read_program.instructions + c) + first_instr_pos : *(ds18b20read_program.instructions + c);
	}

	// enable autopush after 8 bits shifted, set wrap boundaries
	*PIO_SM2_EXECCTRL = (0 << PIO_SM2_EXECCTRL_SIDE_EN_LSB) 
	| ( (ds18b20read_wrap_target + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (ds18b20read_wrap + first_instr_pos) << PIO_SM2_EXECCTRL_WRAP_TOP_LSB);
	*PIO_SM2_SHIFTCTRL = PIO_SM2_SHIFTCTRL_RESET;
	//*PIO_SM2_SHIFTCTRL &= ~(1 << PIO_SM2_SHIFTCTRL_IN_SHIFTDIR_LSB);
	//*PIO_SM2_SHIFTCTRL |= (8 << PIO_SM2_SHIFTCTRL_PUSH_THRESH_LSB) | (1 << PIO_SM2_SHIFTCTRL_AUTOPUSH_LSB);

	*PIO_SM2_PINCTRL = 
      (1 << PIO_SM2_PINCTRL_SET_COUNT_LSB) 
	| (DS18B20_PIN << PIO_SM2_PINCTRL_SET_BASE_LSB) 
    | (DS18B20_PIN << PIO_SM2_PINCTRL_IN_BASE_LSB)
    ;

	// set clock divider
	*PIO_SM2_CLKDIV = DS18B20_CLKDIV_READ << PIO_SM2_CLKDIV_INT_LSB;

	// jump to first instruction
	*PIO_SM2_INSTR = first_instr_pos;

	// enable pio0, sm2
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+2));

	// wait until the rx fifo is not empty anymore
	while ((*PIO_FSTAT & (4 << PIO_FSTAT_RXEMPTY_LSB)) == (4 << PIO_FSTAT_RXEMPTY_LSB));

	rdata = (*PIO_SM2_RXF >> 24)  & 0xFF;
	return rdata;
}

/**
 * @brief initializes a temperature conversion
 * 
 * @return uint16_t 
 */
uint8_t initTempConversion()
{
	uint8_t retcode=0;
	// reset
	retcode = resetDs18b20();

	// skip rom
	writeDs18b20(0xCC);

	// maybe disable interrupt at this point since its timecritical to enable the hard pull
	// convert Temperature
	writeDs18b20(0x44);

	tempreadTicks=getTickValue();
	tempReadState = 1;
	return retcode;
}


uint8_t readTemp(int16_t* res)
{
	if (tempreadTicks > 0 && getTickValue() >= tempreadTicks + 75)
	{
		uint8_t tbyte1, tbyte2, retcode;
		// reset
		retcode = resetDs18b20();
		if (retcode > 0) {return retcode; }
		//skip rom
		writeDs18b20(0xCC);
		// read scratchpad
		writeDs18b20(0xBE);
		tbyte1 = readDs18b20();
		tbyte2 = readDs18b20();
		*res = ((tbyte2 & 0x7) << 8) | (tbyte1);
		if ((tbyte2 & 0xF8)==0xF8)
		{
			*res = -*res;
		}
		tempreadTicks=0;
		retcode = resetDs18b20(); // send reset to terminate reading process
		if (retcode > 0) {return retcode; }
		tempReadState = 0;
		return 0;
	}
	else if (tempreadTicks==0)
	{
		return 2;
	}
	else
	{
		return 4;
	}
}

