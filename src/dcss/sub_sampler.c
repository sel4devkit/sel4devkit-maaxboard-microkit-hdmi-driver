/* This work is Crown Copyright NCSC, 2024. */

#include "sub_sampler.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define COEFF 0x21612161

void write_sub_sampler_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {

	write_register((uint32_t*)(dcss_base + SS_COEFF), COEFF);
	write_register((uint32_t*)(dcss_base + SS_CLIP_CB), 0x3ff << 16); // Set max value for cb clipping function
	write_register((uint32_t*)(dcss_base + SS_CLIP_CR), 0x3ff << 16); // Set max value for cr clipping function

	write_register((uint32_t*)(dcss_base + SS_DISPLAY),
		    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC +
			hdmi_config->V_ACTIVE -1) << 16) |
		       (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
			hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + SS_HSYNC),
		    (((hdmi_config->HSYNC- 1) << 16) | (hdmi_config->HSYNC_POL != 0) << 31 | (hdmi_config->FRONT_PORCH +
			hdmi_config->BACK_PORCH + hdmi_config->HSYNC+ hdmi_config->H_ACTIVE -1)));
	write_register((uint32_t*)(dcss_base + SS_VSYNC),
		    (((hdmi_config->TYPE_EOF +  hdmi_config->VSYNC - 1) << 16) | (hdmi_config->VSYNC_POL != 0) << 31 | (hdmi_config->TYPE_EOF - 1)));
	write_register((uint32_t*)(dcss_base + SS_DE_ULC),
		    ((1 << 31) | (( hdmi_config->VSYNC +hdmi_config->TYPE_EOF + hdmi_config->SOF) << 16) |
		    (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + SS_DE_LRC),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -1) << 16) |
		    (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));

	write_register((uint32_t*)(dcss_base + SS_MODE), 0x0);
	write_register((uint32_t*)(dcss_base + SS_SYS_CTRL), 0x1); // Enable sub sampler
}