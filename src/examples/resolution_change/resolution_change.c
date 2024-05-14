#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "resolution_change.h"
#include "frame_buffer.h"
#include "api.h"

#include "dma_offsets.h"
#include "vic_table.h"

#define API_EXAMPLE_2_SIDE_LENGTH 300
#define RBGA_GREEN 0x00ff0000

int vic_mode = 1;

void init(void) {
	
	init_api();

	for (int i = 0; i < 3; i++) {
		vic_mode = i;
		static_image(init_example);
		reset_static_image(10000);
	}

	reset_api();
}


struct display_config init_example() {

    // TODO: make safer, v mode could exceed array size.

	struct hdmi_data hd;
	hd.FRONT_PORCH = vic_table[vic_mode][FRONT_PORCH];
	hd.BACK_PORCH= vic_table[vic_mode][BACK_PORCH];
	hd.HSYNC = vic_table[vic_mode][HSYNC];
	hd.TYPE_EOF = vic_table[vic_mode][TYPE_EOF];
	hd.SOF = vic_table[vic_mode][SOF];
	hd.VSYNC= vic_table[vic_mode][VSYNC];
	hd.H_ACTIVE = vic_table[vic_mode][H_ACTIVE];
	hd.V_ACTIVE = vic_table[vic_mode][V_ACTIVE]; 
	hd.HSYNC_POL = vic_table[vic_mode][HSYNC_POL];
	hd.VSYNC_POL = vic_table[vic_mode][VSYNC_POL];
	hd.PIXEL_FREQ_KHZ = vic_table[vic_mode][PIXEL_FREQ_KHZ];
	hd.H_BLANK = vic_table[vic_mode][H_BLANK];
	hd.H_TOTAL = vic_table[vic_mode][H_TOTAL];
	hd.VIC_R3_0 = vic_table[vic_mode][VIC_R3_0];
	hd.VIC_PR = vic_table[vic_mode][VIC_PR];
	hd.V_TOTAL = vic_table[vic_mode][V_TOTAL];
	hd.rgb_format = RBGA;
	hd.alpha_enable = ALPHA_OFF;
	hd.mode = STATIC_IMAGE;
	hd.ms_delay = NO_DELAY;

	// Return struct containing the hdmi data and the function to write the frame buffer
	struct display_config dc = {hd, &write_frame_buffer};
	return dc;
}

void write_frame_buffer(struct hdmi_data* hd) { // pass in hdmi config
	
	printf("Writing function api 2\n");
	
	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	uint32_t* frame_buffer_addr = get_active_frame_buffer_uint32();
	
	int width = hd->H_ACTIVE;

	for (int i = 0; i < API_EXAMPLE_2_SIDE_LENGTH; i++) {
		for (int j = 0; j < API_EXAMPLE_2_SIDE_LENGTH; j++) {
			*(frame_buffer_addr++) = RBGA_GREEN;
		}
		frame_buffer_addr += (width-API_EXAMPLE_2_SIDE_LENGTH);
	}
}
