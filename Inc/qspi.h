#ifndef _QSPI_H_
#define _QSPI_H_
#include "stdint.h"

#define QSPI_FLASH_SIZE 0x800000

// status register flags
#define IS25LP064A_SR_WIP ((uint8_t)0x01)

// commands
#define RESET_ENABLE_CMD 0x66
#define RESET_MEMORY_CMD 0x99 /**< & */
#define READ_STATUS_REG_CMD 0x05

void initQspi();



#endif