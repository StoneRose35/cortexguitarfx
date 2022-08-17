/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_PIO_H_
#define _HARDWARE_PIO_H_

#include <stdint.h>
#include <stdbool.h>
typedef unsigned int uint;


typedef struct pio_program {
    const uint16_t *instructions;
    uint8_t length;
    int8_t origin; // required instruction memory origin or -1
} pio_program_t;


/* stubs for being able to consume generated pioasm headers*/
typedef struct 
{
    uint8_t a;
} pio_sm_config;

void sm_config_set_sideset(pio_sm_config* c,int a,bool b,bool d);

void sm_config_set_wrap(pio_sm_config* c,uint32_t wt,uint32_t w);

pio_sm_config pio_get_default_sm_config();

#endif // _PIO_H_
