
#ifdef SIMPLE_TIMERTEST

#include "hardware/regs/addressmap.h"
#include "hardware/regs/pio.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/m0plus.h"
#include "gen/pioprogram.h"
#include "neopixelDriver.h"


#define GPIO13_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO13_CTRL_OFFSET))

#define GPIO_OE ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OE_OFFSET))
#define GPIO_OUT ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))

#define LED_PIN (13)
#define WAITVAL (12000000)

uint32_t sendState;
volatile uint32_t task;

void setupClock();

void led_pin_setup()
{
    //switch on GPIO
    *RESETS |= (1 << 5);
    *RESETS &= ~(1 << 5);
	while ((*RESETS_DONE & (1 << 5)) == 0);

	*GPIO_OE &= ~(1 << LED_PIN);
	*GPIO_OUT &= ~(1 << LED_PIN);

    *GPIO13_CNTR =  5; // function 5 (SIO)
	*GPIO_OE |= (1 << LED_PIN);
}

void set_led()
{
    *(GPIO_OUT+1) = 1 << LED_PIN;
}

void clear_led()
{
    *(GPIO_OUT+2) = 1 << LED_PIN;
}

void isr_pio0_irq0_irq7()
{
	if((*PIO_IRQ & (1 << 1)) == (1 << 1)) // got irq 0, a frame has passed
	{
        set_led();
		*PIO_IRQ = (1 << 1);
	}
}

int main()
{
    uint32_t wait_cnt;

	uint16_t instr_mem_cnt = 0;
	uint16_t instr_offset = 0;
	uint16_t first_instr_pos;

	uint32_t muh=0;
    setupClock();

    led_pin_setup();

    // enable the PIO0 block
	*RESETS |= (1 << RESETS_RESET_PIO0_LSB);
    *RESETS &= ~(1 << RESETS_RESET_PIO0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PIO0_LSB)) == 0);

	// ***********************************************************
	// configure state machine 0 which drives the neopixels
	// ***********************************************************
	
	// switch the neopixel pin to be controlled by the pio0
	*NEOPIXEL_PIN_CNTR =  6; 
	
	first_instr_pos = instr_mem_cnt;
    // enable side-set, set wrap top and wrap bottom
	*PIO_SM0_EXECCTRL = (1 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
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
	*PIO_CTRL |= (1 << PIO_CTRL_SM_ENABLE_LSB+0);
	
    
	first_instr_pos = instr_mem_cnt;
    *PIO_SM1_EXECCTRL = (0 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
	| ( (frametimer_wrap_target + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( (frametimer_wrap + first_instr_pos) << PIO_SM0_EXECCTRL_WRAP_TOP_LSB);

	//  do pull after 32 bits of have been shifted out, disable autopull
	*PIO_SM1_SHIFTCTRL |= (0 << PIO_SM1_SHIFTCTRL_PULL_THRESH_LSB) |(0 << PIO_SM1_SHIFTCTRL_AUTOPULL_LSB);

	// fill in instructions
	// offset the jump instruction by position of the first command since the jump addresses
	// are relative to the program
	for(uint8_t c=0;c < frametimer_program.length;c++){
		*(PIO_INSTR_MEM + instr_mem_cnt++) = (*(frametimer_program.instructions + c) & 0xe000)==0 ?
		 *(frametimer_program.instructions + c) + first_instr_pos : *(frametimer_program.instructions + c);
	}

	// emplicitely jump to first instruction
	*PIO_SM1_INSTR = first_instr_pos;

	//enable interrupt from pio0 sm1
	*PIO_INTE |= (1 << PIO_IRQ0_INTS_SM1_LSB);

	// enable interrupts 7 PIO0_IRQ_0
	*NVIC_ISER = (1 << 7);

    // write the appropriate wait value to the transmit fifo
	*PIO_SM1_TXF = (uint32_t)0x3D0900;

	// start PIO 0, state machine 1
	*PIO_CTRL |= (1 << PIO_CTRL_SM_ENABLE_LSB+1);

    while(1)
    {
        clear_led();
        wait_cnt = 0;
        while (wait_cnt < WAITVAL)
        {
            wait_cnt++;
        }
    }

    return 0;
}


#endif