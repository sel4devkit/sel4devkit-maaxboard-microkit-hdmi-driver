#ifndef __EMPTY_CLIENT_H__
#define __EMPTY_CLIENT_H__

#include "hdmi_data.h"

struct display_config init_empty_client_static_frame_buffer();
void write_empty_client_static_frame_buffer(struct hdmi_data* hd); 

struct display_config init_empty_client_moving_frame_buffer();
void write_empty_client_moving_frame_buffer(struct hdmi_data* hd); 

#endif