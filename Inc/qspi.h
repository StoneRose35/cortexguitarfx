#ifndef _QSPI_H_
#define _QSPI_H_
#include "stdint.h"

#define QSPI_FLASH_SIZE 0x800000

// status register flags
#define IS25LP064A_SR_WIP ((uint8_t)0x01)

#define IS25LP064A_SR_QE 0x40

// commands
#define RESET_ENABLE_CMD 0x66
#define RESET_MEMORY_CMD 0x99 
#define READ_STATUS_REG_CMD 0x05
#define READ_MANUFACTURER_DEVICE_ID 0x90
#define READ_JEDEC_ID 0x9F
#define WRITE_STATUS_REG_CMD 0x01
#define SET_READ_PARAM_REG_CMD 0xC0
#define WRITE_ENABLE_CMD 0x06
#define ENTER_QUAD_CMD 0x35
#define EXIT_QUAD_CMD 0xF5
//#define QUAD_IN_PAGE_PROG_CMD 0x32 
#define PAGE_PROG_CMD 0x02
#define SECTOR_ERASE_QPI_CMD 0x20
#define QUAD_INOUT_FAST_READ_CMD 0xEB   

void initQspi();
void QspiEraseSector(uint32_t address);
void QspiRead(uint32_t address,uint32_t nBytes,uint8_t * data);
void QspiProgramPage(uint32_t address,uint8_t*data);
void setMemoryMappedMode();

#endif