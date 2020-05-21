#ifndef				__DMA__
#define				__DMA__
#include "stm32f4xx_dma.h"
#include "stm32f4xx.h"

extern void MYDMA_Config(DMA_Stream_TypeDef* DMAy_Streamx, uint32_t channel, uint32_t pAddr, uint32_t memAddr, uint32_t dir, uint32_t bufSize);
extern void MY_EnableDMA(DMA_Stream_TypeDef* DMAy_Streamx);


#endif
