#include "stdint.h"
#include "hardware/address_mapped.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"

#include "drivers/irq.h"

void setInterruptPriority(uint8_t interruptNr,uint8_t priority)
{
    uint8_t prioRegisterNr = interruptNr >> 2;
    *(NVIC_IPR0 + prioRegisterNr) = priority << (((interruptNr & 0x3)<<3) + 6);
}