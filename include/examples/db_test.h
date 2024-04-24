#ifndef __DB_TEST_H__
#define __DB_TEST_H__

#include "hdmi_data.h"

struct display_config init_example();
void write_frame_buffer(struct hdmi_data* hd); 

#endif