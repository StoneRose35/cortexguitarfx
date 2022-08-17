#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "pwm.h"

void initPwm()
{
        // pwm
    *RESETS |= (1 << RESETS_RESET_PWM_LSB);
    *RESETS &= ~(1 << RESETS_RESET_PWM_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PWM_LSB)) == 0);
}