
.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb
  

.align 2

// delay function for the display driver (ssd1306)
// manually tuned to work @180MHz and 2.8125MHz SPI Clock Frequency
.global short_nop_delay
.type short_nop_delay,%function
.thumb_func
short_nop_delay:
push {r0}
ldr r0,=0x17
short_delay_l1:
nop
sub r0,r0,#1
cmp r0,#0
bne short_delay_l1
pop {r0}
bx lr

.section .RamFunc.convolve
// computes the sum of the products of two float vectors
// a[0]*b[0] + a[1]*b[1] + a[2]*b[2] .... 
// r0: pointer to coefficients, r1: pointer to data, r2: pointer offset
//.section .data.RamFunc
.global convolve
.type convolve,%function
.thumb_func
convolve:
vpush {s1,s2}
push {r4,r5}
mov r4,r1
add r4,r2
ldr r5,=0x40
vldr.32 s0,[r0,#4]
vldr.32 s1,[r4]
vmul.f32 s0,s0,s1
sub r5,#1
cmp r5,#0
beq conv_loop_end
conv_loop:
add r2,r2,#4
orr r2,r2,#63
mov r4,r1
add r4,r2
vldr.32 s1,[r0,#4]
vldr.32 s2,[r4]
vmla.f32 s0,s1,s2
sub r2,#1
cmp r2,#0
bne conv_loop
conv_loop_end:
vpop {s1,s2}
pop {r4,r5}
bx lr
