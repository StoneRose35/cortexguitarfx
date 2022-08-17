

#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/m0plus.h"
#include <stdlib.h>
#include "systemClock.h"
#include "uart.h"
#include "stringFunctions.h"
#include "system.h"

#define LED_PIN (25) // 13 on rp2040 feather, 25 on rpi pico boards

#define GPIO13_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO13_CTRL_OFFSET))
#define GPIO25_CNTR ((volatile uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO25_CTRL_OFFSET))
#define RESETS ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_OFFSET))
#define RESETS_DONE ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_DONE_OFFSET))

#define GPIO_OE ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OE_OFFSET))
#define GPIO_OUT ((volatile uint32_t*)(SIO_BASE + SIO_GPIO_OUT_OFFSET))

#define NVIC_ISER ((volatile uint32_t*)(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET))




volatile int testarray[256];
volatile uint32_t answer_to_all_questions_of_the_universe = 42;
void blinkSetup();
void _sr35_delay(uint32_t val);

uint8_t context = (1 << CONTEXT_USB);

void notmain()
{
	blinkSetup();
	while(1)
	{
		*GPIO_OUT |= (1 << LED_PIN);
		_sr35_delay(6500000UL);
		*GPIO_OUT &= ~(1 << LED_PIN);
		_sr35_delay(6500000UL);			
	}	
}

void blinkSetup()
{
    //switch on GPIO
    *RESETS &= ~(1 << 5);
	while ((*RESETS_DONE & (1 << 5)) == 0);

	*GPIO_OE &= ~(1 << LED_PIN);
	*GPIO_OUT &= ~(1 << LED_PIN);

    *GPIO25_CNTR =  5; // function 5 (SIO)
	*GPIO_OE |= (1 << LED_PIN);
}

