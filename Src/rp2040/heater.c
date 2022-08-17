#include <stdint.h>
#include "hardware/regs/pwm.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/addressmap.h"
#include "hardware/rp2040_registers.h"
#include "heater.h"


/**
 * @brief Set the value of the heater, a value from 0 to 1023 is expected
 * 
 * @param val 
 */
void setHeater(uint16_t val)
{
    *PWM_CH1_CC = (val << 6);
}

void initHeater()
{
    *PWM_CH1_TOP = 0xFFFF;
    *PWM_CH1_CC = 0x0; 
    *PWM_CH1_CSR |= (1 << PWM_CH1_CSR_EN_LSB);
    *GPIO_OE |= (1 << HEATER);
    *HEATER_PIN_CNTR = 4;
}
