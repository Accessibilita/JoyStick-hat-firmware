#include "dma.h"

void MX_DMA_Init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6U, 0U);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}
