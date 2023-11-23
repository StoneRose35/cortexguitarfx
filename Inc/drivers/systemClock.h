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

#endif /* SYSTEMCLOCK_H_ */

#endif
