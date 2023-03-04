#include "dma.h"
#include "stm32f446/stm32f446xx.h"

void initDMA()
{
    RCC->AHB1ENR |= (1 << RCC_AHB1ENR_DMA2EN_Pos) | (1 << RCC_AHB1ENR_DMA1EN_Pos);
}