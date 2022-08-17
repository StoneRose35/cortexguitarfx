
.syntax unified
.cpu cortex-m0plus
.thumb
/*
taken from the rp2040 sdk
 */

.macro function_decl x
.global \x
.type \x,%function
.thumb_func
\x:
.endm

// void FUNC_NAME(__aeabi_cfrcmple)(float, float)         reversed 3-way (<, =, ?>) compare [1], result in PSR ZC flags
.align 2
function_decl __aeabi_cfrcmple
    push {r0-r2, lr}
    eors r0, r1
    eors r1, r0
    eors r0, r1
    b __aeabi_cfcmple_guts

// NOTE these share an implementation as we have no excepting NaNs.
// void FUNC_NAME(__aeabi_cfcmple)(float, float)         3-way (<, =, ?>) compare [1], result in PSR ZC flags
// void FUNC_NAME(__aeabi_cfcmpeq)(float, float)         non-excepting equality comparison [1], result in PSR ZC flags
.align 2
function_decl __aeabi_cfcmple
function_decl __aeabi_cfcmpeq
    push {r0-r2, lr}

__aeabi_cfcmple_guts:
    lsls r2,r0,#1
    lsrs r2,#24
    beq 1f
    cmp r2,#0xff
    bne 2f
    lsls r2, r0, #9
    bhi 3f
1:
    lsrs r0,#23     @ clear mantissa if denormal or infinite
    lsls r0,#23
2:
    lsls r2,r1,#1
    lsrs r2,#24
    beq 1f
    cmp r2,#0xff
    bne 2f
    lsls r2, r1, #9
    bhi 3f
1:
    lsrs r1,#23     @ clear mantissa if denormal or infinite
    lsls r1,#23
2:
    movs r2,#1      @ initialise result
    eors r1,r0
    bmi 2f          @ opposite signs? then can proceed on basis of sign of x
    eors r1,r0      @ restore y
    bpl 1f
    cmp r1,r0
    pop {r0-r2, pc}
1:
    cmp r0,r1
    pop {r0-r2, pc}
2:
    orrs r1, r0     @ handle 0/-0
    adds r1, r1     @ note this always sets C
    beq 3f
    mvns r0, r0     @ carry inverse of r0 sign
    adds r0, r0
3:
    pop {r0-r2, pc}


// int FUNC_NAME(__aeabi_fcmpeq)(float, float)         result (1, 0) denotes (=, ?<>) [2], use for C == and !=
//float_wrapper_section __aeabi_fcmpeq
.align 2
function_decl __aeabi_fcmpeq
    push {lr}
    bl __aeabi_cfcmpeq
    beq 1f
    movs r0, #0
    pop {pc}
1:
    movs r0, #1
    pop {pc}

// int FUNC_NAME(__aeabi_fcmplt)(float, float)         result (1, 0) denotes (<, ?>=) [2], use for C <
//float_wrapper_section __aeabi_fcmplt
.align 2
function_decl __aeabi_fcmplt
    push {lr}
    bl __aeabi_cfcmple
    sbcs r0, r0
    pop {pc}

// int FUNC_NAME(__aeabi_fcmple)(float, float)         result (1, 0) denotes (<=, ?>) [2], use for C <=
//float_wrapper_section __aeabi_fcmple
.align 2
function_decl __aeabi_fcmple
    push {lr}
    bl __aeabi_cfcmple
    bls 1f
    movs r0, #0
    pop {pc}
1:
    movs r0, #1
    pop {pc}

// int FUNC_NAME(__aeabi_fcmpge)(float, float)         result (1, 0) denotes (>=, ?<) [2], use for C >=
//float_wrapper_section __aeabi_fcmpge
.align 2
function_decl __aeabi_fcmpge
    push {lr}
    // because of NaNs it is better to reverse the args than the result
    bl __aeabi_cfrcmple
    bls 1f
    movs r0, #0
    pop {pc}
1:
    movs r0, #1
    pop {pc}

// int FUNC_NAME(__aeabi_fcmpgt)(float, float)         result (1, 0) denotes (>, ?<=) [2], use for C >
//float_wrapper_section __aeabi_fcmpgt
function_decl __aeabi_fcmpgt
    push {lr}
    // because of NaNs it is better to reverse the args than the result
    bl __aeabi_cfrcmple
    sbcs r0, r0
    pop {pc}