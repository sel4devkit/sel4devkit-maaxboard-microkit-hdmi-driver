#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "api_example_1.h"
#include "hdmi_data.h"
#include "frame_buffer.h"

#include "dma_offsets.h"

#include "vic_table.h"

struct display_config init_example1() {
	
	// Initialise the hdmi data with custom values
	struct hdmi_data hd;//{1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_OFF, DB_OFF, NO_DELAY};

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
	hd.alpha_toggle = ALPHA_OFF;
	hd.db_toggle = DB_OFF;
	hd.ms_delay = NO_DELAY;



	// Return struct containing the hdmi data and the function to write the frame buffer
	struct display_config dc = {hd, &write_api_example_1_frame_buffer};
	return dc;
}


void write_api_example_1_frame_buffer(struct hdmi_data* hd) { // pass in hdmi config
	
	printf("Writing function api 1\n");
	
	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	uint8_t* frame_buffer_addr = get_frame_buffer_uint8();
	

//	(*frame_buffer_addr--);

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
		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
		starting with a 0 alhpa increasing every 3 pixels.
	*/ 
//	for (int i = 0; i < height; i++) { 
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
	//}
}



// void api_example2() {
	
// 	// Loop through the first three vic table values to display an image of the same size in three different resolutions
// 	for (int i = 0; i < 3; i++) {
// 		vic_table_api_example(i);
// 	}
// }

// void vic_table_api_example(int v_mode) {

// 	// Initialise the data from the predefined vic_table
// 	hdmi_config->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
// 	hdmi_config->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
// 	hdmi_config->HSYNC = vic_table[v_mode][HSYNC];
// 	hdmi_config->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
// 	hdmi_config->SOF = vic_table[v_mode][SOF];
// 	hdmi_config->VSYNC= vic_table[v_mode][VSYNC];
// 	hdmi_config->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
// 	hdmi_config->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
// 	hdmi_config->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
// 	hdmi_config->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
// 	hdmi_config->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
// 	hdmi_config->H_BLANK = vic_table[v_mode][H_BLANK];
// 	hdmi_config->H_TOTAL = vic_table[v_mode][H_TOTAL];
// 	hdmi_config->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
// 	hdmi_config->VIC_PR = vic_table[v_mode][VIC_PR];
// 	hdmi_config->V_TOTAL = vic_table[v_mode][V_TOTAL];
// 	hdmi_config->rgb_format = RBGA;
// 	hdmi_config->alpha_toggle = ALPHA_OFF;
// 	hdmi_config->db_toggle = DB_OFF;
// 	hdmi_config->ms_delay = NO_DELAY;

// 	// Write a square of a fixed size to the frame buffer at current resolution 
// 	write_api_example_2_frame_buffer();

// 	// Send the hdmi data to the dcss PD to initialise the DCSS
// 	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));

// 	// How long the example is shown for
// 	ms_delay(10000);
	
// 	// Clear the frame buffer
// 	clear_frame_buffer();

// 	// Reset the DCSS for next example to avoid strange visual effects caused by prewriting the buffer.
// 	microkit_notify(55); 
// }


// void api_example3() {

// 	// Initialise the vic mode with custom values
// 	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, RGBA, ALPHA_OFF, DB_ON, NO_DELAY};
// 	*hdmi_config = v;

// 	// Prewrite the buffer before display configuration
// 	write_api_example_3_frame_buffer_64();
// 	write_frame_buffer = &write_api_example_3_frame_buffer_64;

// 	// Send the hdmi data to the dcss PD to initialise the DCSS
// 	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));
	
// }

// void api_example4() {	// example set up - 

// 	int v_mode = 0; // here for debugging to try different display configs

// 	hdmi_config->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
// 	hdmi_config->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
// 	hdmi_config->HSYNC = vic_table[v_mode][HSYNC];
// 	hdmi_config->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
// 	hdmi_config->SOF = vic_table[v_mode][SOF];
// 	hdmi_config->VSYNC= vic_table[v_mode][VSYNC];
// 	hdmi_config->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
// 	hdmi_config->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
// 	hdmi_config->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
// 	hdmi_config->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
// 	hdmi_config->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
// 	hdmi_config->H_BLANK = vic_table[v_mode][H_BLANK];
// 	hdmi_config->H_TOTAL = vic_table[v_mode][H_TOTAL];
// 	hdmi_config->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
// 	hdmi_config->VIC_PR = vic_table[v_mode][VIC_PR];
// 	hdmi_config->V_TOTAL = vic_table[v_mode][V_TOTAL];
// 	hdmi_config->rgb_format = RGBA;
// 	hdmi_config->alpha_toggle = ALPHA_OFF;
// 	hdmi_config->db_toggle = DB_ON;
// 	hdmi_config->ms_delay = 1000;
	
// 	// Prewrite the buffer before display configuration
// 	write_api_example_4_frame_buffer_64();
// 	write_frame_buffer = &write_api_example_4_frame_buffer_64;

// 	// Send the hdmi data to the dcss PD to initialise the DCSS
// 	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));
// }


// void api_example5() {


// 	int v_mode = 0; // here for debugging to try different display configs

// 	hdmi_config->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
// 	hdmi_config->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
// 	hdmi_config->HSYNC = vic_table[v_mode][HSYNC];
// 	hdmi_config->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
// 	hdmi_config->SOF = vic_table[v_mode][SOF];
// 	hdmi_config->VSYNC= vic_table[v_mode][VSYNC];
// 	hdmi_config->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
// 	hdmi_config->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
// 	hdmi_config->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
// 	hdmi_config->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
// 	hdmi_config->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
// 	hdmi_config->H_BLANK = vic_table[v_mode][H_BLANK];
// 	hdmi_config->H_TOTAL = vic_table[v_mode][H_TOTAL];
// 	hdmi_config->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
// 	hdmi_config->VIC_PR = vic_table[v_mode][VIC_PR];
// 	hdmi_config->V_TOTAL = vic_table[v_mode][V_TOTAL];
// 	hdmi_config->rgb_format = RGBA;
// 	hdmi_config->alpha_toggle = ALPHA_ON;
// 	hdmi_config->db_toggle = DB_OFF;
// 	hdmi_config->ms_delay = NO_DELAY;
	

// 	// Prewrite the buffer before display configuration
// 	write_api_example_5_frame_buffer_64();
// 	write_frame_buffer = &write_api_example_5_frame_buffer_64;

// 	// Send the hdmi data to the dcss PD to initialise the DCSS
// 	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));
	
// }

// void write_api_example_1_frame_buffer() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	
// 	//printf("writing frame buffer...\n");
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE;

// 	int first_quarter = width * 0.25;
// 	int second_quarter = width * 0.5;
// 	int third_quarter = width * 0.75;
// 	int alpha = 0;
// 	int j_pos = width_offset;


// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
// 		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
// 		starting with a 0 alhpa increasing every 3 pixels.
// 	*/ 
// 	for (int i = 0; i < height; i++) { 
// 		for (int j = 0; j < width; j++) {
			
// 			// reset alpha for each colour bar
// 			if (j % first_quarter == 0) {
// 				alpha = 0;
// 			}
			
// 			if (j < first_quarter)
// 			{
// 				*(frame_buffer_addr++) = 0xff; 
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = alpha;
// 			}
// 			else if (j < second_quarter)
// 			{
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = 0xff;
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = alpha;
// 			}
// 			else if (j < third_quarter)
// 			{
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = 0x00;
// 				*(frame_buffer_addr++) = 0xff;
// 				*(frame_buffer_addr++) = alpha;
// 			}
// 			else {
// 				*(frame_buffer_addr++) = 0xff;
// 				*(frame_buffer_addr++) = 0xff;
// 				*(frame_buffer_addr++) = 0xff;
// 				*(frame_buffer_addr++) = alpha;
// 			}

// 			if (j %3 == 0) {
// 				alpha++;
// 			}
// 		}
// 	}
// }

// void write_api_example_2_frame_buffer() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}

// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

// 	int width = hdmi_config->H_ACTIVE;
	
// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha.
// 		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle.
// 	*/ 
// 	for (int i = 0; i < API_EXAMPLE_2_SIDE_LENGTH; i++) {
// 		for (int j = 0; j < API_EXAMPLE_2_SIDE_LENGTH; j++) {
// 			*(frame_buffer_addr++) = 0xff;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0xff;
// 		}
// 		frame_buffer_addr += 4*(width-API_EXAMPLE_2_SIDE_LENGTH);
// 	}
// }

// void write_api_example_3_frame_buffer() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	

// 	// get frame_buffer will just return a pointer to this at whatever integer amount. 
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint32_t* frame_buffer_addr = (uint32_t*)(dma_base + *frame_buffer_addr_offset);

// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE;
// 	int first_quarter = width * 0.25;
// 	int second_quarter = width * 0.5;
// 	int third_quarter = width * 0.75;
// 	int j_pos = width_offset;

// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
// 		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
// 		starting with a 0 alhpa increasing every 3 pixels.
// 	*/ 
// 	for (int i = 0; i < height; i++) {
// 		for (int j = 0; j < width; j++) {
			
// 			if (j_pos < first_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_RED; 
// 			}
// 			else if (j_pos < second_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_GREEN; 
// 			}
// 			else if (j_pos < third_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_BLUE; 
// 			}
// 			else {
// 				*(frame_buffer_addr++) = RGBA_WHITE; 
// 			}
			
// 			j_pos++;
// 			if (j_pos == width)
// 			{
// 				j_pos = 0;
// 			}
// 		}
// 	}

// 	width_offset += 1;
// 	if (width_offset == width) {
// 		width_offset = 0;
// 	}

// 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// 	if (ctx_ld_enable == 1) {
		
// 		// Call the context loader to signify the buffer has finished being written to.
// 		microkit_notify(52);	
// 	}
// 	else {
// 		ctx_ld_enable = 1;
// 	}
// }


// void write_api_example_3_frame_buffer_64() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint64_t* frame_buffer_addr = (uint64_t*)(dma_base + *frame_buffer_addr_offset);

// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE/2;
// 	int first_quarter = width * 0.25;
// 	int second_quarter = width * 0.5;
// 	int third_quarter = width * 0.75;
// 	int j_pos = width_offset;

// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
// 		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
// 		starting with a 0 alhpa increasing every 3 pixels.
// 	*/ 
// 	for (int i = 0; i < height; i++) {
// 		for (int j = 0; j < width; j++) {
			
// 			if (j_pos < first_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_RED_64; 
// 			}
// 			else if (j_pos < second_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_GREEN_64; 
// 			}
// 			else if (j_pos < third_quarter)
// 			{
// 				*(frame_buffer_addr++) = RGBA_BLUE_64; 
// 			}
// 			else {
// 				*(frame_buffer_addr++) = RGBA_WHITE_64; 
// 			}
			
// 			j_pos++;
// 			if (j_pos == width)
// 			{
// 				j_pos = 0;
// 			}
// 		}
// 	}

// 	width_offset += 1;
// 	if (width_offset == width) {
// 		width_offset = 0;
// 	}

// 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// 	if (ctx_ld_enable == 1) {
		
// 		// Call the context loader to signify the buffer has finished being written to.
// 		microkit_notify(52);	
// 	}
// 	else {
// 		ctx_ld_enable = 1;
// 	}
// }

// int current_fb = 0; // global here just for the purpose of this example 


// void write_api_example_4_frame_buffer_64() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint64_t* frame_buffer_addr = (uint64_t*)(dma_base + *frame_buffer_addr_offset);

// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE/2; // /2 as 64bits are being written 

// 	// for current testing purposes both frame buffers are pre written.
// 	if (current_fb == 0) {
// 		printf("writing frame buffer 1 as red.\n");
// 		for (int i = 0; i < height; i++) {
// 			for (int j = 0; j < width; j++) {
// 				*(frame_buffer_addr++) = RGBA_RED_64; 
// 			}
// 		}

// 		frame_buffer_addr = (uint64_t*)(dma_base + FRAME_BUFFER_TWO_OFFSET);

// 		printf("writing frame buffer 2 as blue.\n");
// 		for (int i = 0; i < height; i++) {
// 			for (int j = 0; j < width; j++) {
// 				*(frame_buffer_addr++) = RGBA_BLUE_64; 
// 			}
// 		}
// 		current_fb++;
// 	}
	
// 	//  It is doing it like this just for the purpose of the example to keep the same format of notifying once the buffers written. 

// 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// 	if (ctx_ld_enable == 1) {
		
// 		// Call the context loader to signify the buffer has finished being written to.
// 		microkit_notify(52);	
// 	}
// 	else {
// 		ctx_ld_enable = 1;
// 	}
// }

// void write_api_example_5_frame_buffer_64() {
	
// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint64_t* frame_buffer_addr = (uint64_t*)(dma_base + *frame_buffer_addr_offset);

// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE/2; // /2 as 64bits are being written 

// 	// for current testing purposes both frame buffers are pre written.
// 	if (current_fb == 0) {
// 		printf("writing frame buffer 1 as red.\n");
// 		for (int i = 0; i < height; i++) {
// 			for (int j = 0; j < width/2; j++) {
// 				*(frame_buffer_addr++) = RGBA_RED_64_5; 
// 			}
// 			frame_buffer_addr += (width-(width/2));
// 		}

// 		frame_buffer_addr = (uint64_t*)(dma_base + FRAME_BUFFER_TWO_OFFSET);

// 		printf("writing frame buffer 2 as blue.\n");
// 		for (int i = 0; i < height; i++) {
// 			for (int j = 0; j < width; j++) {
// 				*(frame_buffer_addr++) = RGBA_BLUE_64_5; 
// 			}
// 		}
// 		current_fb++;
// 	}
	

// 	//  It is doing it like this just for the purpose of the example to keep the same format of notifying once the buffers written. 

// 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// 	if (ctx_ld_enable == 1) {
		
// 		// Call the context loader to signify the buffer has finished being written to.
// 		microkit_notify(52);	
// 	}
// 	else {
// 		ctx_ld_enable = 1;
// 	}
// }




// // void write_api_example_3_frame_buffer_mem_cpy() {
	
// // 	if (hdmi_config == NULL){
// // 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// // 		return;
// // 	}
	
// // 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// // 	uint32_t* frame_buffer_start_addr = (uint32_t*)(dma_base + *frame_buffer_addr_offset);

// // 	int height = hdmi_config->V_ACTIVE;
// // 	int width = hdmi_config->H_ACTIVE;


// // 	// I could draw this all first, then i could memcpy from the other buffer but shifted along by 1.
// // 	// then another cpy to get the last pixel of the row as the first pixel
// // 	// I then would have an entire range 

// // 	// for each pixel it is 4 bytes (4* 8 bits = 32 bits)
// // 	// So the copy would be ((width-1)*4)
// // 	// the source addr would be uint32_t frame_buffr++ (so its one along)

// // 	// the current frame buffer being displayed
// // 	int current_frame_buffer = (*frame_buffer_addr_offset == FRAME_BUFFER_ONE_OFFSET) ? FRAME_BUFFER_TWO_OFFSET : FRAME_BUFFER_ONE_OFFSET;
// // 	uint32_t* current_frame_buffer_addr = (uint32_t*)(dma_base + current_frame_buffer);

// // 	memcpy((void*)current_frame_buffer_addr+1, (void*)(dma_base + current_frame_buffer), (width-1) / 4);
// // 	*frame_buffer_start_addr = *(current_frame_buffer_addr + width - 1);
	
// // 	uint32_t* frame_buffer_addr = frame_buffer_start_addr;

// // 	int width_in_bytes = width/4;
// // 	for (int i = 1; i < height; i++)
// // 	{
// // 		memcpy((void*) (frame_buffer_start_addr+width), (void*)(frame_buffer_start_addr), width_in_bytes);
// // 		frame_buffer_addr = frame_buffer_addr+width;
// // 	}
	

// // 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// // 	if (ctx_ld_enable == 1) {
		
// // 		// Call the context loader to signify the buffer has finished being written to.
// // 		microkit_notify(52);	
// // 	}
// // 	else {
// // 		ctx_ld_enable = 1;
// // 	}
// // }



// // void write_api_example_3_frame_buffer() {
	
// // 	if (hdmi_config == NULL){
// // 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// // 		return;
// // 	}
	
// // 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// // 	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

// // 	uint8_t* frame_buffer_start_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);
// // 	printf("frame buffer addr before write = %p\n", frame_buffer_addr);

// // 	int height = hdmi_config->V_ACTIVE;
// // 	int width = hdmi_config->H_ACTIVE;
// // 	int first_quarter = width * 0.25;
// // 	int second_quarter = width * 0.5;
// // 	int third_quarter = width * 0.75;
// // 	int alpha = 0;
// // 	int j_pos = width_offset;

// // 	/*
// // 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// // 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// // 		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
// // 		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// // 		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
// // 		starting with a 0 alhpa increasing every 3 pixels.
// // 	*/ 
// // 	for (int i = 0; i < height; i++) {
// // 		for (int j = 0; j < width; j++) {
			
// // 			if (j_pos < first_quarter)
// // 			{
// // 				*(frame_buffer_addr++) = 0xff; 
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = alpha;
// // 			}
// // 			else if (j_pos < second_quarter)
// // 			{
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = 0xff;
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = alpha;
// // 			}
// // 			else if (j_pos < third_quarter)
// // 			{
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = 0x00;
// // 				*(frame_buffer_addr++) = 0xff;
// // 				*(frame_buffer_addr++) = alpha;
// // 			}
// // 			else {
// // 				*(frame_buffer_addr++) = 0xff;
// // 				*(frame_buffer_addr++) = 0xff;
// // 				*(frame_buffer_addr++) = 0xff;
// // 				*(frame_buffer_addr++) = alpha;
// // 			}
			
// // 			j_pos++;
// // 			if (j_pos == width)
// // 			{
// // 				j_pos = 0;
// // 			}
// // 		}
// // 	}

// // 	width_offset += 2;
// // 	if (width_offset == width) {
// // 		width_offset = 0;
// // 	}

// // 	//seL4_ARM_VSpace_Invalidate_Data(3, (long)frame_buffer_start_addr, (long)frame_buffer_addr);
// // 	//int ret = seL4_ARM_VSpace_CleanInvalidate_Data(2, 0, (long)CTX_LD_DMA_SIZE);
// // 	//printf("Return from cache flush = %d\n", ret);
// // 	printf("frame buffer start addr = %p\n", frame_buffer_start_addr);
// // 	printf("frame buffer addr after write = %p\n", frame_buffer_addr);
// // 	//ms_delay(5000);

// // 	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
// // 	if (ctx_ld_enable == 1) {
		
// // 		// Call the context loader to signify the buffer has finished being written to.
// // 		microkit_notify(52);	
// // 	}
// // 	else {
// // 		ctx_ld_enable = 1;
// // 	}
// // }

// void clear_frame_buffer() { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled.
	
// 	printf("clearing buffer\n");
// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
// 	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

// 	if (hdmi_config == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}
	
// 	int height = hdmi_config->V_ACTIVE;
// 	int width = hdmi_config->H_ACTIVE;

// 	for (int i = 0; i < height; i++) {
// 		for (int j = 0; j < width; j++) {
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 		}
// 	}
// }