/* This work is Crown Copyright NCSC, 2024. */

#include "scaler.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>

#define RUN_EN 0
#define ENABLE_REPEAT 4


void write_scaler_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {

	write_register((uint32_t*)(dcss_base  + SCALE_SRC_DATA_CTRL), 0x0); // Must be initialised
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_FORMAT), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_DST_FORMAT), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_LUMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_CHROMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_DST_CHROMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_V_LUMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_LUMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_V_CHROMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_CHROMA_INC), 0x00002000);

	// Scaler coeffecients
	write_register((uint32_t*)(dcss_base  + 0x1c0c0), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c140), 0x00000000); // Must be initialised
	write_register((uint32_t*)(dcss_base  + 0x1c180), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c1c0), 0x00000000); // Must be initialised
	write_register((uint32_t*)(dcss_base  + 0x1c000), 0x00000011);
	write_register((uint32_t*)(dcss_base  + 0x1c020), ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));

	uint32_t* scale_ctrl = (uint32_t*)(dcss_base + SCALE_CTRL);
	*scale_ctrl = set_bit(*scale_ctrl, RUN_EN);
	*scale_ctrl = set_bit(*scale_ctrl, ENABLE_REPEAT);
}