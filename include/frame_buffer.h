#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <stdio.h> // find out which one is needed. 
#include <stddef.h>
#include <stdlib.h>

#include "hdmi_data.h"


uint8_t* get_frame_buffer_uint8();
uint64_t* get_frame_buffer_uint64();
void clear_current_frame_buffer(struct hdmi_data* hd);

#endif