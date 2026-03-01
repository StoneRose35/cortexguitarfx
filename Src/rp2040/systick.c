
#include "drivers/systick.h"
#include "system.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"

static volatile uint32_t ticks=0;
static volatile uint32_t ticks_c1=0;
extern volatile uint32_t encoderSpeed;
extern volatile int32_t encoderVal;
volatile int32_t encoderValOldSpeedMesurement=0;
void isr_c0_systick()
{
    ticks++;
}

void isr_c1_systick()
{
    int32_t currentCnt = encoderVal;
    ticks_c1++;
    if (currentCnt > encoderValOldSpeedMesurement)
    {
        encoderSpeed = currentCnt - encoderValOldSpeedMesurement;
    }
    else
    {
        encoderSpeed = encoderValOldSpeedMesurement - currentCnt;
    }
    encoderValOldSpeedMesurement = currentCnt;
}

uint32_t getTickValue()
{
    if (*SIO_CPUID == 0)
    {
        return ticks;
    }
    return ticks_c1;
}

void initSystickTimer()
{
    *M0PLUS_SYST_CVR = F_SYS/100;
    *M0PLUS_SYST_RVR = F_SYS/100; // generate a systick every 10ms
    *M0PLUS_SYST_CSR = (1 << M0PLUS_SYST_CSR_CLKSOURCE_LSB) | (1 << M0PLUS_SYST_CSR_ENABLE_LSB) | (1 << M0PLUS_SYST_CSR_TICKINT_LSB);
}


void stopSystickTimer()
{
    *M0PLUS_SYST_CVR = F_SYS/100;
    *M0PLUS_SYST_RVR = F_SYS/100;
    *M0PLUS_SYST_CSR &= ~(1 << M0PLUS_SYST_CSR_ENABLE_LSB);
}

void waitSysticks(uint32_t dticks)
{
    uint32_t t0;
    uint32_t tlim;
    t0 = getTickValue();
    tlim = t0+dticks;
    while (getTickValue() <= tlim);
}
