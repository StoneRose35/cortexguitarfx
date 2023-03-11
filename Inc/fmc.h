#ifndef _FMC_H_
#define _FMC_H_

#include "stdint.h"

#define FMC_SDRAM_CMD_NORMAL_MODE               ((uint8_t)0x0U)
#define FMC_SDRAM_CMD_CLK_ENABLE                ((uint8_t)0x1U)
#define FMC_SDRAM_CMD_PALL                      ((uint8_t)0x2U)
#define FMC_SDRAM_CMD_AUTOREFRESH_MODE          ((uint8_t)0x3U)
#define FMC_SDRAM_CMD_LOAD_MODE                 ((uint8_t)0x4U)
#define FMC_SDRAM_CMD_SELFREFRESH_MODE          ((uint8_t)0x5U)
#define FMC_SDRAM_CMD_POWERDOWN_MODE            ((uint8_t)0x6U)

#define SDRAM_MODEREG_BURST_LENGTH_2 ((1 << 0))
#define SDRAM_MODEREG_BURST_LENGTH_4 ((1 << 1))

#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((0 << 3))

#define SDRAM_MODEREG_CAS_LATENCY_3 ((1 << 4) | (1 << 5))

#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ()

#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((1 << 9))
#define SDRAM_MODEREG_WRITEBURST_MODE_PROG_BURST ((0 << 9))


void initFmcSdram();

#endif