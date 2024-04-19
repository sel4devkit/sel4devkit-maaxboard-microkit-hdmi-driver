
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "api_example_4.h"

#include "frame_buffer.h"
#include "dma_offsets.h"
#include "vic_table.h"

#define RGBA_RED_64 0x000000ff000000ff
#define RGBA_GREEN_64 0x0000ff000000ff00
#define RGBA_BLUE_64 0x00ff000000ff0000
#define RGBA_WHITE_64 0x00ffffff00ffffff

int current_fb = 0; // to stop it from redrawing the buffers

struct display_config init_example_4()  {	// example set up - 

	int v_mode = 0; // here for debugging to try different display configs

   	struct hdmi_data hd;

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
	hd.rgb_format = RGBA;
	hd.alpha_enable = ALPHA_OFF;
	hd.db_enable = CTX_LD;
	hd.ms_delay = 1000;
    
    // Return struct containing the hdmi data and the function to write the frame buffer
	struct display_config dc = {hd, &write_api_example_4_frame_buffer};
	return dc;
}

void write_api_example_4_frame_buffer(struct hdmi_data* hd) {
	
	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}
	
    uint64_t* frame_buffer_addr = get_active_frame_buffer_uint64();

	int height = hd->V_ACTIVE;
	int width = hd->H_ACTIVE/2; // /2 as 64bits are being written 

	// for current testing purposes both frame buffers are pre written.
	if (current_fb == 0) {
		printf("writing frame buffer 1 as red.\n");
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				*(frame_buffer_addr++) = RGBA_RED_64; 
			}
		}

		frame_buffer_addr = get_cache_frame_buffer_uint64();

		printf("writing frame buffer 2 as blue.\n");
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				*(frame_buffer_addr++) = RGBA_BLUE_64; 
			}
		}
		current_fb++;
	}
}