
#ifdef SIMPLE_NEOPIXEL

#include "hardware/regs/addressmap.h"
#include "hardware/regs/pio.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/sio.h"
#include "gen/pioprogram.h"
#include "neopixelDriver.h"

#define GPIO13_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO13_CTRL_OFFSET))
#define RESETS ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_OFFSET))
#define RESETS_DONE ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_DONE_OFFSET))

#define GPIO_OE ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OE_OFFSET))
#define GPIO_OUT ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))

#define LED_PIN (13)
#define WAITVAL (120000)

void colorUpdate(RGB * color,uint32_t phase);
void setupClock();
volatile uint32_t task;

void colorUpdate(RGB * color,uint32_t phase)
{
	if (phase < 0x100)
	{
		color->r = 0xFF;
		color->g = phase & 0xFF;
		color->b = 0x00;
	}
	else if (phase < 0x200)
	{
		color->r = 0x1FF - phase;
		color->g = 0xFF;
		color->b = 0x00;
	}
	else if (phase < 0x300)
	{
		color->r = 0x00;
		color->g = 0xFF;
		color->b = phase - 0x200;
	}
	else if (phase < 0x400)
	{
		color->r = 0x00;
		color->g = 0x3FF - phase;
		color->b = 0xFF;
	}
	else if (phase < 0x500)
	{
		color->r = phase - 0x400;
		color->g = 0x00;
		color->b = 0xFF;
	}
	else
	{
		color->r = 0xFF;
		color->g = 0x00;
		color->b = 0x5FF - phase;
	}
}

uint8_t put_val_blocking(uint32_t val)
{
    uint8_t retval=0;
    volatile uint32_t fstat_val = *PIO_FSTAT & (1 << PIO_FSTAT_TXEMPTY_LSB);
    volatile uint32_t cmpval = (1 << PIO_FSTAT_TXEMPTY_LSB);

    if (fstat_val==cmpval)
    {
        retval = 1;
    }
    while ((*PIO_FSTAT & (1 << PIO_FSTAT_TXFULL_LSB)) == (1 << PIO_FSTAT_TXFULL_LSB));
    *PIO_SM0_TXF = val;
    return retval;
}


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


int main()
{

    setupClock();

    led_pin_setup();

	uint16_t instr_mem_cnt = 0;
	uint16_t first_instr_pos;

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
	*PIO_SM0_EXECCTRL = (0 << PIO_SM0_EXECCTRL_SIDE_EN_LSB) 
	| ( ws2812_wrap_target + first_instr_pos << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
	| ( ws2812_wrap + first_instr_pos << PIO_SM0_EXECCTRL_WRAP_TOP_LSB);

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

	// check all instructions
	for (uint8_t cc=0;cc< ws2812_program.length;cc++)
	{
		*PIO_SM0_INSTR = cc + first_instr_pos;
		uint8_t muh = 1;
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

	// test send red
	*PIO_SM0_TXF = ((60 << 24) | (250 << 16) | (60 << 8));


    uint32_t cnt = 0;
    uint8_t txstate = 0;
    uint32_t cnt2 = 0;
    RGB clr;

    while(1)
    {
        colorUpdate(&clr,cnt);
        txstate = 0;
        txstate += put_val_blocking((clr.g << 24)|(clr.r << 16)|(clr.b << 8));
        cnt++;
        if (cnt == 0x600)
        {
            cnt = 0;
        }
        //txstate > 0 ? set_led(): clear_led();
        cnt2 = 0;
        set_led();
        while(cnt2 < WAITVAL >> 1)
        {
            cnt2++;
        }
        clear_led();
        cnt2 = 0;
        while(cnt2 < WAITVAL >> 1)
        {
            cnt2++;
        }


    }

    return 0;
}

#endif