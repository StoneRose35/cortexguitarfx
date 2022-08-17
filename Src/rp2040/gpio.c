#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/rp2040_registers.h"
#include "hardware/regs/resets.h"
#include "gpio.h"


void setAsOuput(uint8_t pinNr)
{
    *GPIO_OE |= (1 << pinNr);
    uint32_t* pinCtrl = ((uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*pinNr));
    uint32_t* padCtrl = ((uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*pinNr));
    *padCtrl &= ~(1 << PADS_BANK0_GPIO0_OD_LSB);
    *pinCtrl = 5;
}

void setAsInput(uint8_t pinNr, uint8_t pullState)
{
    uint32_t* pinCtrl = ((uint32_t*)(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8*pinNr));
    *pinCtrl = 5;   
    *GPIO_OE &= ~(1 << pinNr);
    uint32_t* padCtrl = ((uint32_t*)(PADS_BANK0_BASE + PADS_BANK0_GPIO0_OFFSET + 4*pinNr));
    switch (pullState)
    {
    case PULL_STATE_NONE:
        *padCtrl &= ~((1 << PADS_BANK0_GPIO0_PDE_LSB) | (1 << PADS_BANK0_GPIO0_PUE_LSB));
        break;
    case PULL_STATE_DOWN:
        *padCtrl &= ~((1 << PADS_BANK0_GPIO0_PUE_LSB));
        *padCtrl |= (1 << PADS_BANK0_GPIO0_PDE_LSB);
    case PULL_STATE_UP:
        *padCtrl &= ~((1 << PADS_BANK0_GPIO0_PDE_LSB));
        *padCtrl |= (1 << PADS_BANK0_GPIO0_PUE_LSB);
    default:
        break;
    }
    *padCtrl |= (1 << PADS_BANK0_GPIO0_IE_LSB);
}

uint8_t readPin(uint8_t pinNr)
{
    if ((*GPIO_IN & (1 << pinNr))!= 0)
    {
        return 1;
    }
    return 0;
}

void setPin(uint8_t pinNr,uint8_t value)
{
    if (value != 0)
    {
        *(GPIO_OUT+1) = (1 << pinNr);
    }
    else
    {
        *(GPIO_OUT+2) = (1 << pinNr);
    }
}

void initGpio()
{
	//NEVER-EVER RESET GPIO PINS!!! (nothing works anymore)
    *RESETS &= ~(1 << RESETS_RESET_IO_BANK0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_IO_BANK0_LSB)) == 0);

	// get pads out of reset
    *RESETS |= (1 << RESETS_RESET_PADS_BANK0_LSB); 
	*RESETS &= ~(1 << RESETS_RESET_PADS_BANK0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PADS_BANK0_LSB)) == 0);
}
