#ifndef _PI_REGULATOR_H_
#define _PI_REGULATOR_H_

#include <stdint.h>
 #include "services/triopsBreederService.h"

uint16_t getRegulatedHeaterValue(TriopsControllerType * controller,uint16_t measuredTemp);

#endif