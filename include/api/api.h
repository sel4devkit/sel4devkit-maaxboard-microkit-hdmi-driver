#ifndef __API_H__
#define __API_H__

#include "hdmi_data.h"

void static_image(struct display_config (*init_func)());
void moving_image(struct display_config (*init_func)());
void run_examples();

#endif