
#include "multicore.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/m0plus.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
extern uint32_t __StackBottom;


static void __attribute__ ((naked))  jumpIntoCore1Main()
{
    __asm__("pop {pc}");
}

void startCore1(void(*secondMain)())
{
    uint32_t * stackPtr = &__StackBottom;
    stackPtr[0] = (uint32_t)secondMain;
    const uint32_t cmd_sequence[] = {0, 0, 1, (uintptr_t) *M0PLUS_VTOR, (uintptr_t) stackPtr, (uintptr_t)jumpIntoCore1Main};
    
    uint32_t nvic_current = *NVIC_ISER;
    *NVIC_ICER = (1 << 15); // clear proc0 SIO interrupt

    uint8_t seq = 0;
    do {
        uint32_t cmd = cmd_sequence[seq];
        // we drain before sending a 0
        if (!cmd) {
            while ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB)) == (1 << SIO_FIFO_ST_VLD_LSB))
            {
                (void)*SIO_FIFO_RD;
            }
            __sev(); // core 1 may be waiting for fifo space
        }
        while ((*SIO_FIFO_ST & ( 1 << SIO_FIFO_ST_RDY_LSB)) == 0);
        *SIO_FIFO_WR = cmd;

        __sev();

        while ((*SIO_FIFO_ST & ( 1 << SIO_FIFO_ST_VLD_LSB)) == 0);
        uint32_t response = *SIO_FIFO_RD; 
        // move to next state on correct response otherwise start over
        seq = cmd == response ? seq + 1 : 0;
    } while (seq < 6);

    if ((nvic_current & (1 << 15)) == (1 << 15))
    {
        *NVIC_ISER = (1 << 15);
    }
}

void __sev()
{
    __asm__("sev");
}