#include "stm32h750/stm32h750xx.h"
#include "core.h"
#include "globalConfig.h"
#include "system.h"
extern uint32_t task;

void enableFpu()
{
	SCB->CPACR |= ((3UL << 10*2) |             /* set CP10 Full Access               */
                 (3UL << 11*2)  );           /* set CP11 Full Access               */
}

/* define all memory regions except SDRAM as non-cacheable */
uint32_t configureAndEnableMPU()
{
    if ((MPU->TYPE & 0x0000FF00)==0) // no mpu present
    {
        return 1;
    }
    //configure region 0 (SRAM D1)
    MPU->RNR = 0;
    MPU->RBAR = 0x24000000;
    MPU->RASR = (0 << MPU_RASR_XN_Pos) | // no execute never
                (3 << MPU_RASR_AP_Pos) | // full access
                (4 << MPU_RASR_TEX_Pos) | (0 << MPU_RASR_C_Pos) | (0 << MPU_RASR_B_Pos) | // normal, non-cacheable
                (18 << MPU_RASR_SIZE_Pos) | //size: 2^(18+1) -> 512k
                (1 << MPU_RASR_ENABLE_Pos) // enable region                
                ;
    // configure region 1 (SRAM D2)
    MPU->RNR = 1;
    MPU->RBAR = 0x30000000;
    MPU->RASR = (0 << MPU_RASR_XN_Pos) | // no execute never
                (3 << MPU_RASR_AP_Pos) | // full access
                (4 << MPU_RASR_TEX_Pos) | (0 << MPU_RASR_C_Pos) | (0 << MPU_RASR_B_Pos) | // normal, non-cacheable
                (18 << MPU_RASR_SIZE_Pos) | //size: 2^(18+1) -> 512k
                (1 << MPU_RASR_ENABLE_Pos) // enable region                
                ;   

    // configure region 3 (SDRAM)
    MPU->RNR = 2;
    MPU->RBAR = 0xC0000000;
    MPU->RASR = (0 << MPU_RASR_XN_Pos) | // no execute never
            (3 << MPU_RASR_AP_Pos) | // full access
            (1 << MPU_RASR_TEX_Pos) | (1 << MPU_RASR_C_Pos) | (1 << MPU_RASR_B_Pos) | // normal, outer and inner write-back, write and read allocate
            (25 << MPU_RASR_SIZE_Pos) | //size: 2^(25+1) -> 64M
            (1 << MPU_RASR_ENABLE_Pos) // enable region                
            ;   


    // enable the mpu
    MPU->CTRL = (1 << MPU_CTRL_PRIVDEFENA_Pos) // use default map for privileg accesses
            | (0 << MPU_CTRL_HFNMIENA_Pos) // disable mpu for hardfaults etc
            | (1 << MPU_CTRL_ENABLE_Pos); // enable the mpu
    
    
    return 0;
}



