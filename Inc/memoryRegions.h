#ifndef MEMORY_REGIONS_H
#define MEMORY_REGIONS_H

#define __LOBYTE(a) (((uint16_t)a & 0xff))
#define __HIBYTE(a) (((uint16_t)a & 0xFF) >> 8)
#define __QSPI_CODE __attribute__ ((section (".qspi_code")))
#define __QSPI_DATA __attribute__ ((section (".qspi_data")))
#define __QSPI_DATA_FAST __attribute__ ((section (".qspi_fast_data")))
#define __ITCM_CODE  __attribute__ ((section (".qspi_itcm_code")))
#define __ITCM_CODE_FLASH  __attribute__ ((section (".flash_itcm_code")))
#define __SDRAM_BSS __attribute__ ((section (".sdram_bss")))
#define __DTCM_BSS __attribute__ ((section (".dtcm_bss")))
#define __DTCM_DATA __attribute__ ((section (".dtcm_data")))
#define __RAMFUNC __attribute__ ((section (".RamFunc")))


#endif // MEMORY_REGIONS_H