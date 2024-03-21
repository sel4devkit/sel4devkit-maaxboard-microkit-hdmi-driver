#ifndef __DMA_OFFSETS_H__
#define __DMA_OFFSETS_H__


// These all need redefining 
#define FRAME_BUFFER_SIZE 1280 * 720 * 3
#define CTX_LD_DMA_SIZE FRAME_BUFFER_SIZE * 4 
#define FRAME_BUFFER_TWO_OFFSET FRAME_BUFFER_SIZE * 3
#define CTX_LD_DB_ONE_ADDR FRAME_BUFFER_SIZE * 2
#define CTX_LD_DB_TWO_ADDR (FRAME_BUFFER_SIZE * 2) + 1000000


#endif