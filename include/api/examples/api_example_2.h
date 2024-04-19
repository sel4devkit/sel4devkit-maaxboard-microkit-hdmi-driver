#ifndef __API_EXAMPLE_2_H__
#define __API_EXAMPLE_2_H__

#include "hdmi_data.h"

struct display_config init_example_2();
void write_api_example_2_frame_buffer(struct hdmi_data* hd);
void set_example_2_vic_mode(int vm);

#endif