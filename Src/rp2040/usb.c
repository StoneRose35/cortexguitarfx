#include "stdint.h"
#include "hardware/regs/clocks.h"
#include "hardware/regs/pll.h"
#include "hardware/regs/addressmap.h"
#include "hardware/rp2040_registers.h"
#include "systemClock.h"

/**
 * @brief initializes the usb hardware
 *        restarts the usb pll of not locked
 * 
 */
void initUSB()
{
    if ((PLL_USB->cs & (1 << PLL_CS_LOCK_LSB))==0)
    {
        initUsbPll();
    }
    *CLK_USB_CTRL &= ~(0x7 << CLOCKS_CLK_USB_CTRL_AUXSRC_LSB);
    *CLK_USB_CTRL |= (1 << CLOCKS_CLK_USB_CTRL_ENABLE_LSB);
}