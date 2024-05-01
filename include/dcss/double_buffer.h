#ifndef __DOUBLE_BUFFER_H__
#define __DOUBLE_BUFFER_H__

#include <stdint.h>

#include "hdmi_data.h"

// for testing
void change_buffer();
void change_buffer_manually();
void run_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* active_frame_buffer_offset, uint32_t* cache_frame_buffer_offset);
void init_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* active_frame_buffer_offset, uint32_t* cache_frame_buffer_offset);

void init_context_loader_flip(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config);
void run_context_loader_flip(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config);

#endif