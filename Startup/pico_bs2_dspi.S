/**
 * bootloader enabling the double speed spi used for testing on the rp2040 feather
 */
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/ssi.h"
#include "hardware/regs/pads_qspi.h"

#define CMD_READ 0x03
#define CMD_READ_DOUBLE 0x3b

#define SSI_ENR (XIP_SSI_BASE + SSI_SSIENR_OFFSET)
#define SSI_BAUDR (XIP_SSI_BASE + SSI_BAUDR_OFFSET)
#define SSI_CNTR0 (XIP_SSI_BASE + SSI_CTRLR0_OFFSET)
#define SSI_CNTR1 (XIP_SSI_BASE + SSI_CTRLR1_OFFSET)
#define SSI_SPI_CNTR0 (XIP_SSI_BASE + SSI_SPI_CTRLR0_OFFSET)


#define PICO_FLASH_SPI_CLKDIV 4

//register offsets 
#define XIP_CMD 24
#define ADDR_L 2
#define INSTR_L 8
#define SSI_TMOD 8
#define SSI_DFS_32 16 
#define SSI_FRF 21
#define SSI_WAIT_CYCLES 11
#define TRANS_TYPE 0
#define XIP_CMD 24


.syntax unified
.cpu cortex-m0plus
.thumb

.section .boot2_code, "ax"
.align 2


/* trying to initialize  the xip flash using default single channel spi mode, can be used for the rp2040 feather*/

push {lr}

    // Set pad configuration:
    // - SCLK 8mA drive, no slew limiting
    // - SDx disable input Schmitt to reduce delay

    ldr r3, =PADS_QSPI_BASE
    movs r0, #(2 << PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_LSB | PADS_QSPI_GPIO_QSPI_SCLK_SLEWFAST_BITS)
    str r0, [r3, #PADS_QSPI_GPIO_QSPI_SCLK_OFFSET]
    ldr r0, [r3, #PADS_QSPI_GPIO_QSPI_SD0_OFFSET]
    movs r1, #PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS
    bics r0, r1
    str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD0_OFFSET]
    str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD1_OFFSET]
    str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD2_OFFSET]
    str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD3_OFFSET]

// disable ssi
ldr r3, =SSI_ENR
movs r4, #0
str r4,[r3]

// set baud rate
ldr r3,=SSI_BAUDR
movs r4,PICO_FLASH_SPI_CLKDIV // set clock, division of 1 since the ROSC 
          // is supposed to run at ~6.5MHz and the GD25Q06CSIG
          // supports up to 120MHz
str r4,[r3]

// set SSI_CNTR0
ldr r3, =SSI_CNTR0
ldr r4, =((31 << SSI_DFS_32) | (3 << SSI_TMOD) | (1 << SSI_FRF))
str r4,[r3]

// set SSI_SPI_CNTR0
ldr r3, =SSI_SPI_CNTR0
ldr r4, =((CMD_READ_DOUBLE << XIP_CMD) | (6 << ADDR_L) | (8 << SSI_WAIT_CYCLES) | (2 << INSTR_L))
str r4,[r3]

// read one data frame at the time
ldr r3, =SSI_CNTR1
movs r4, #0x0
str r4, [r3]

// enable ssi
ldr r3, =SSI_ENR
movs r4, #1
str r4,[r3]



// jump off to second stage bootloader, which is at the 0x100-boundary from XIP (0x1000 0000)
// initialize the stack pointer
ldr r1, =(XIP_BASE + 0x100)
ldr r2,[r1]
msr msp, r2

// branch to _start_c_runtime
ldr r1, =(XIP_BASE + 0x104) 
ldr r2,[r1]
bx r2
