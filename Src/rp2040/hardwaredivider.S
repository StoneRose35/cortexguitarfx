.syntax unified
.cpu cortex-m0plus
.thumb
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"

.align 2

.global __aeabi_idiv
.type __aeabi_idiv,%function
.thumb_func
__aeabi_idiv:
    ldr r3, =SIO_BASE
    str r0, [r3, #SIO_DIV_SDIVIDEND_OFFSET]
    str r1, [r3, #SIO_DIV_SDIVISOR_OFFSET]
    // delay 8 cycles
    b 1f
    1:  b 1f
    1:  b 1f
    1:  b 1f
    1:
    ldr r1, [r3, #SIO_DIV_REMAINDER_OFFSET]
    ldr r0, [r3, #SIO_DIV_QUOTIENT_OFFSET]
    bx lr


.global __aeabi_uidiv
.type __aeabi_uidiv,%function
.thumb_func
__aeabi_uidiv:
    ldr r3, =SIO_BASE
    str r0, [r3, #SIO_DIV_UDIVIDEND_OFFSET]
    str r1, [r3, #SIO_DIV_UDIVISOR_OFFSET]
    // delay 8 cycles
    b 1f
    1:  b 1f
    1:  b 1f
    1:  b 1f
    1:
    ldr r1, [r3, #SIO_DIV_REMAINDER_OFFSET]
    ldr r0, [r3, #SIO_DIV_QUOTIENT_OFFSET]
    bx lr
    