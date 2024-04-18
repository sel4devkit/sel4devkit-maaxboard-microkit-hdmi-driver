#include "frame_buffer.h"
#include <microkit.h>
#include "dma_offsets.h"

uintptr_t dma_base; 

uint8_t* get_frame_buffer_uint8(){
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);    // make another define, one for visible buffer and one for the backup.
	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);
	return frame_buffer_addr;
}

uint64_t* get_frame_buffer_uint64(){
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
	uint64_t* frame_buffer_addr = (uint64_t*)(dma_base + *frame_buffer_addr_offset);
	return frame_buffer_addr;
}


void clear_current_frame_buffer(struct hdmi_data* hd) {
	
	printf("clearing buffer\n");
	
	uint64_t* frame_buffer_addr = get_frame_buffer_uint64();

	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}
	
	int height = hd->V_ACTIVE;
	int width = hd->H_ACTIVE/2;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			*(frame_buffer_addr++) = 0x00;
		}
	}
}


