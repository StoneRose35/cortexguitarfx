#include "audio/firFilter.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/dma.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"



void isr_c1_sio_irq_proc1_irq16() // only fires when a fir computation has to be made
{
    int16_t firstHalfOut;
    FirFilterType**core1FirData;
    if ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB))!= 0)
    {
        core1FirData = (FirFilterType**)*SIO_FIFO_RD;
        firstHalfOut = processFirstHalf(*core1FirData);
        *SIO_FIFO_WR = firstHalfOut;
    }
    else if (((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_ROE_LSB)) != 0) || ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_WOF_LSB)) != 0))
    {
        *SIO_FIFO_ST = (1 << 2);
    }
}