/*
 * systemClock.c
 *
 *  Created on: 21.08.2021
 *      Author: philipp
 */
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/xosc.h"
#include "hardware/regs/clocks.h"
#include "hardware/regs/pll.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "systemClock.h"
#include "system.h"



void setupClock()
{
	

	// explicitely define startup delay cycles
	// 281 results from (12MHz * 6ms)/256
	*XOSC_STARTUP = 281;
	*XOSC_CTRL_ENABLE = (XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB)  | XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ;
	while ((*XOSC_STATUS & (1 << XOSC_STATUS_STABLE_LSB))==0);

	// switch to XOSC for ref 
	*CLK_REF_CTRL |= (0x02 << 0);

	
	// de-reset sys pll
	*RESETS &= ~(1 << RESETS_RESET_PLL_SYS_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PLL_SYS_LSB)) == 0);

	//lauch PLL
	//refdiv 1
	PLL_SYS->cs |= (1 << 0) ;
	//vco runs at 12MHz*FEEDBK
	PLL_SYS->fbdiv = FEEDBK;

	// wwitch on pll itself and vco
	PLL_SYS->pwr &= ~((1 << PLL_PWR_PD_LSB) | (1 << PLL_PWR_VCOPD_LSB));

	//wait until lock is achieved
	while((PLL_SYS->cs & (1 << PLL_CS_LOCK_LSB))==0);

	// system clock runs at f_vco/postdiv1/postdiv2
	PLL_SYS->prim = (POSTDIV1 << PLL_PRIM_POSTDIV1_LSB) | (POSTDIV2 << PLL_PRIM_POSTDIV2_LSB);

	// enable post divider
	PLL_SYS->pwr &= ~(1 << PLL_PWR_POSTDIVPD_LSB);

	// switch system clock to pll (src to aux which is sys pll by default)
	*CLK_SYS_CTRL |= (0x01 << 0);

	// enable peripheral clock
	*CLK_PERI_CTRL |= (1 << CLOCKS_CLK_PERI_CTRL_ENABLE_LSB);
}

void initUsbPll()
{
	// de-reset usb pll
	*RESETS |= (1 << RESETS_RESET_PLL_USB_LSB);
	*RESETS &= ~(1 << RESETS_RESET_PLL_USB_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_PLL_USB_LSB)) == 0);
	PLL_USB->cs |= (1 << 0) ;
	//vco runs at 12MHz*FEEDBK
	PLL_USB->fbdiv = FEEDBK;

	// wwitch on pll itself and vco
	PLL_USB->pwr &= ~((1 << PLL_PWR_PD_LSB) | (1 << PLL_PWR_VCOPD_LSB));

	//wait until lock is achieved
	while((PLL_USB->cs & (1 << PLL_CS_LOCK_LSB))==0);

	// usb clock runs at f_vco/postdiv1/postdiv2
	PLL_USB->prim = (POSTDIV1_USB << PLL_PRIM_POSTDIV1_LSB) | (POSTDIV2_USB << PLL_PRIM_POSTDIV2_LSB);

	// enable post divider
	PLL_USB->pwr &= ~(1 << PLL_PWR_POSTDIVPD_LSB);
}

