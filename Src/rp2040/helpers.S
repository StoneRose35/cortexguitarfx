
.syntax unified
.cpu cortex-m0plus
.thumb
  

.align 2

// delay function for the display driver (ssd1306)
// manually tuned to work @480MHz and 7.5MHz SPI Clock Frequency
.global short_nop_delay
.type short_nop_delay,%function
.thumb_func
short_nop_delay:
push {r0}
ldr r0,=62
short_delay_l1:
nop
subs r0,r0,#1
cmp r0,#0
bne short_delay_l1
pop {r0}
bx lr

// a short nop based delay, number of cycles in in first argument (r0)
.global nop_wait
.type nop_wait,%function
.thumb_func
nop_wait:
push {r1}
mov r1,r0
nop_wait_l1:
nop
subs r1,r1,#1
cmp r1,#0
bne nop_wait_l1
pop {r1}
bx lr

