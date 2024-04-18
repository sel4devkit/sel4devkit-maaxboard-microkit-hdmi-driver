#ifndef __DOUBLE_BUFFER_H__
#define __DOUBLE_BUFFER_H__

#include <stdint.h>

#include "hdmi_data.h"

// for testing
void change_buffer();
void change_buffer_manually();
void run_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* current_frame_buffer_offset);
void init_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* current_frame_buffer_offset);

#endif