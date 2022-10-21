
.syntax unified
.cpu cortex-m4
;.fpu softvfp
.thumb
  

.align 2

; delay function for the display driver (ssd1306)
; manually tuned to work @180MHz and 2.8125MHz SPI Clock Frequency
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
