#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/pio.h"
#include "hardware/regs/io_bank0.h"
#include "gen/pio0_pio.h"
#include "remoteSwitch.h"
#include "hardware/rp2040_registers.h"

void initRemoteSwitch()
{
	// handover pin control to pio
	*REMOTESWITCH_PIN_CNTR = 6;

	// same only pin for sideset and set (to be able to configure the pin as output) 
	*PIO_SM3_PINCTRL = 
      (1 << PIO_SM3_PINCTRL_SIDESET_COUNT_LSB) 
	| (REMOTESWITCH_PIN << PIO_SM3_PINCTRL_SIDESET_BASE_LSB) |
	  (1 << PIO_SM3_PINCTRL_SET_COUNT_LSB) | 
	  (REMOTESWITCH_PIN << PIO_SM3_PINCTRL_SET_BASE_LSB)
    ;

    // set pindirs, 1
    *PIO_SM3_INSTR = 0xe081;

	// shift in from the left
	*PIO_SM3_SHIFTCTRL &= ~(1 << PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_LSB);
}


void remoteSwitchOn()
{
	uint32_t cmd = (0b10101110 << 24) | (0b11010110 << 16) | (0b01101100 << 8);
	sendRemoteSwitchCommand(cmd);
}

void remoteSwitchOff()
{
	uint32_t cmd = (0b10101000 << 24) | (0b00101111 << 16) | (0b10111100 << 8);
	sendRemoteSwitchCommand(cmd);
}

void sendRemoteSwitchCommand(uint32_t cmd)
{
    uint16_t instr_mem_cnt = REMOTESWITCH_INSTR_MEM_OFFSET;
	uint16_t first_instr_pos;

	//disable pio0, sm3
	*PIO_CTRL &= ~(1 << (PIO_CTRL_SM_ENABLE_LSB+3));

	// drain tx fifo
	while ((*PIO_FSTAT & (8 << PIO_FSTAT_TXEMPTY_LSB)) == 0)
	{
		*PIO_SM3_INSTR = 0x88; //pull
	}

	//load remoteswitch program
	first_instr_pos = instr_mem_cnt;
	for(uint8_t c=0;c < remoteswitch_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(remoteswitch_program.instructions + c) & 0xe000)==0 ?
		 *(remoteswitch_program.instructions + c) + first_instr_pos : *(remoteswitch_program.instructions + c);
	}

	// set wrap boundaries
	*PIO_SM3_EXECCTRL = (0 << PIO_SM3_EXECCTRL_SIDE_EN_LSB) 
	| ( (remoteswitch_wrap_target + first_instr_pos) << PIO_SM3_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (remoteswitch_wrap + first_instr_pos) << PIO_SM3_EXECCTRL_WRAP_TOP_LSB);
	//*PIO_SM3_SHIFTCTRL = PIO_SM3_SHIFTCTRL_RESET;
	//*PIO_SM3_SHIFTCTRL |= (24 << PIO_SM3_SHIFTCTRL_PULL_THRESH_LSB) | (1 << PIO_SM3_SHIFTCTRL_AUTOPULL_LSB);

	// set clock divider
	*PIO_SM3_CLKDIV = REMOTESWITCH_CLKDIV << PIO_SM3_CLKDIV_INT_LSB;

	// jump to first instruction
	*PIO_SM3_INSTR = first_instr_pos;

	// put command to write into the tx fifo (three times)
	*PIO_SM3_TXF = cmd;
	*PIO_SM3_TXF = cmd;
	*PIO_SM3_TXF = cmd;

	// clear the tx stall flag
	*PIO_FDEBUG = 8 << PIO_FDEBUG_TXSTALL_LSB;

	// enable pio0, sm3
	*PIO_CTRL |= (1 << (PIO_CTRL_SM_ENABLE_LSB+3));

	// wait until tx stall flag is on
	//while((*PIO_FDEBUG & (8 <<PIO_FDEBUG_TXSTALL_LSB)) == 0);
}