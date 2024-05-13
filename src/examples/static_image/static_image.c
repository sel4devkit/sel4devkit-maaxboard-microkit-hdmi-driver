#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "static_image.h"
#include "frame_buffer.h"
#include "api.h"

#include "dma_offsets.h"
#include "vic_table.h"

#include "api.h"

void init(void) {
	
	init_api();
	static_image(init_example);
	reset_static_image(50000);
}

struct display_config init_example() {
	
	// Initialise the hdmi data with custom values
	struct hdmi_data hd;//{1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_OFF, STATIC_IMAGE, NO_DELAY};

	int v_mode = 1;

	hd.FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
	hd.BACK_PORCH= vic_table[v_mode][BACK_PORCH];
	hd.HSYNC = vic_table[v_mode][HSYNC];
	hd.TYPE_EOF = vic_table[v_mode][TYPE_EOF];
	hd.SOF = vic_table[v_mode][SOF];
	hd.VSYNC= vic_table[v_mode][VSYNC];
	hd.H_ACTIVE = vic_table[v_mode][H_ACTIVE];
	hd.V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
	hd.HSYNC_POL = vic_table[v_mode][HSYNC_POL];
	hd.VSYNC_POL = vic_table[v_mode][VSYNC_POL];
	hd.PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
	hd.H_BLANK = vic_table[v_mode][H_BLANK];
	hd.H_TOTAL = vic_table[v_mode][H_TOTAL];
	hd.VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
	hd.VIC_PR = vic_table[v_mode][VIC_PR];
	hd.V_TOTAL = vic_table[v_mode][V_TOTAL];
	hd.rgb_format = RBGA;
	hd.alpha_enable = ALPHA_ON;
	hd.mode = STATIC_IMAGE;
	hd.ms_delay = NO_DELAY;

	// Return struct containing the hdmi data and the function to write the frame buffer
	struct display_config dc = {hd, &write_frame_buffer};
	return dc;
}


void write_frame_buffer(struct hdmi_data* hd) { // pass in hdmi config
	
	printf("Writing function api 1\n");
	
	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	uint8_t* frame_buffer_addr = get_active_frame_buffer_uint8();
	
	int height = hd->V_ACTIVE;
	int width = hd->H_ACTIVE;
	int first_quarter = width * 0.25;
	int second_quarter = width * 0.5;
	int third_quarter = width * 0.75;
	int alpha = 0;

	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_enable. With this option turned on, this example will display each colour bar
		starting with a 0 alhpa increasing every 3 pixels. It is much quicker to write 32 or 64bit colours - see other api examples for this.
	*/ 
	for (int i = 0; i < height; i++) { 
		for (int j = 0; j < width; j++) {
			
			// reset alpha for each colour bar
			if (j % first_quarter == 0) {
				alpha = 0;
			}

			if (j < first_quarter)
			{
				*(frame_buffer_addr++) = 0xff; 
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = alpha;
			}
			else if (j < second_quarter)
			{
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = alpha;
			}
			else if (j < third_quarter)
			{
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = alpha;
			}
			else {
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = alpha;
			}

			if (j %3 == 0) {
				alpha++;
			}
		}
	}
}
