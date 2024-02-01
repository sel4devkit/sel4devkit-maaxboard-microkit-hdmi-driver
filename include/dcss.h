#ifndef __DCSS_H__
#define __DCSS_H__

#include <display_timing.h>

struct hdmi_information {
    uint32_t base_address;
    int vic_mode;
    int horizontal_pulse_polarity;
    int vertical_pulse_polarity;
    struct display_timing timings;
};


void hdmi_set_timings(struct hdmi_information* hdmi_info);
void write_dcss_memory_registers(struct hdmi_information* hdmi_info);
void init_dcss();
void init_hdmi();

#endif