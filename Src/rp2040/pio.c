#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"

void initPio()
{
	// enable the PIO0 block
	*RESETS |= (1 << RESETS_RESET_PIO0_LSB);
    *RESETS &= ~(1 << RESETS_RESET_PIO0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PIO0_LSB)) == 0);

	// enable the PIO1 block
	*RESETS |= (1 << RESETS_RESET_PIO1_LSB);
    *RESETS &= ~(1 << RESETS_RESET_PIO1_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PIO1_LSB)) == 0);
}