/*
 * systemClock.h
 *
 *  Created on: Aug 22, 2021
 *      Author: philipp
 */
#ifndef SYSTEMCLOCK_H_
#define SYSTEMCLOCK_H_

#include "systemChoice.h"
#include <stdint.h>
#ifdef HARDWARE


void setupClock();
void initUsbPll();

void _sr35_delay(uint32_t);
#endif /* SYSTEMCLOCK_H_ */

#endif
