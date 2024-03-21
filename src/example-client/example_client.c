#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>

#include "timer.h"
#include "hdmi_data.h"

#include "dma_offsets.h"

#define API_EXAMPLE_2_SIDE_LENGTH 300

uintptr_t dma_base; // This should be called the DMA pool
uintptr_t frame_buffers[2];
uintptr_t frame_buffer_start_addr;
uintptr_t timer_base;

struct hdmi_data *v_data = NULL;
int current_buffer_index = 0;
int width_offset = 0;

void api_example1();
void api_example2();
void api_example3();
void clear_frame_buffer(int width, int height);
void vic_table_api_example(int v_mode);
void write_api_example_1_frame_buffer(int width, int height);
void write_api_example_3_frame_buffer(int width, int height, int ctx_ld_enable);
void write_api_example_2_frame_buffer(int width);

void init(void) {
	
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);

	// Set up location of both frame buffers
	frame_buffers[0] = dma_base; 
	frame_buffers[1] = dma_base + FRAME_BUFFER_TWO_OFFSET; 

	// Set the current frame buffer
	frame_buffer_start_addr = frame_buffers[0];
	
	// Allocate memory to hold the vic data
	v_data = malloc(sizeof(struct hdmi_data));


	api_example1(); // Display 4 colour bars RGB and white split evenly across the screen with a custom configuration.

	api_example2();

	api_example3();

	//free(v_data); (This will need to be freed at some point)
	printf("Finished Init Client \n");
}


void
notified(microkit_channel ch) {

	switch (ch) {
        case 52:
			current_buffer_index = current_buffer_index == 1 ? 0 : 1; // switch the current buffer index
			frame_buffer_start_addr = frame_buffers[current_buffer_index];
			write_api_example_3_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE, 1);
			break;
		default:
			printf("Unexpected channel id: %d in example_client::notified() \n", ch);
	}
}

void api_example1() {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_ON, DB_OFF};
	*v_data = v;

	// pre write the buffer before display configuration
	write_api_example_1_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE); // this will be defined in a separate file. The frame buffer ADDR will be psased in

	// Send the vic mode data to the dcss PD and init the dcss.
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0)); // This funciton is no longer needed as the v_data can just be passed straight in.
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46); // This will just need the V data passed in but it will call init_dcss().

	ms_delay(10000);

	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}

void api_example2() {
	
	// Loop through the first three vic table values to display an image of the same size in three different resolutions
	for (int i = 0; i < 3; i++) {
		vic_table_api_example(i);
	}
}

void vic_table_api_example(int v_mode) {

	// Initialise the data from the predefined vic_table
	v_data->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
	v_data->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
	v_data->HSYNC = vic_table[v_mode][HSYNC];
	v_data->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
	v_data->SOF = vic_table[v_mode][SOF];
	v_data->VSYNC= vic_table[v_mode][VSYNC];
	v_data->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
	v_data->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
	v_data->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
	v_data->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
	v_data->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
	v_data->H_BLANK = vic_table[v_mode][H_BLANK];
	v_data->H_TOTAL = vic_table[v_mode][H_TOTAL];
	v_data->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
	v_data->VIC_PR = vic_table[v_mode][VIC_PR];
	v_data->V_TOTAL = vic_table[v_mode][V_TOTAL];
	v_data->rgb_format = RBGA;
	v_data->alpha_toggle = ALPHA_OFF;
	v_data->db_toggle = DB_OFF;

	// Write a square of a fixed size to the frame buffer at current resolution 
	write_api_example_2_frame_buffer(v_data->H_ACTIVE);

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);

	ms_delay(10000);
	
	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}


void api_example3() {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_OFF, DB_ON};
	*v_data = v;

	// pre write the buffer before display configuration (This would probably be done in a separate PD?)
	write_api_example_3_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE, 0);

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);
}

void write_api_example_1_frame_buffer(int width, int height) {
	
	printf("writing frame buffer...\n");
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	int first_quarter = width * 0.25;
	int second_quarter = width * 0.5;
	int third_quarter = width * 0.75;
	int alpha = 0;
	int j_pos = width_offset;

	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
		starting with a 0 alhpa increasing every 3 pixels.
	*/ 
	for (int i = 0; i < height; i++) { 
		for (int j = 0; j < width; j++) { // to rotate i to have a counter for the length of the width. But the j will start at an offset and wrap around. Once the counter has been reached then it will be finished
			
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

void write_api_example_2_frame_buffer(int width) {
	
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	
	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha.
		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_toggle.
	*/ 
	for (int i = 0; i < API_EXAMPLE_2_SIDE_LENGTH; i++) {
		for (int j = 0; j < API_EXAMPLE_2_SIDE_LENGTH; j++) {
			*(frame_buffer_addr++) = 0xff;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0xff;
		}
		frame_buffer_addr += 4*(width-API_EXAMPLE_2_SIDE_LENGTH);
	}
}

void write_api_example_3_frame_buffer(int width, int height, int ctx_ld_enable) {
	
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	printf("EXAMPLE 3 !!!!!!!!!!!! frame buffer addr = %p\n", frame_buffer_addr);
	printf("EXAMPLE 3 height %d width %d\n", height, width);

	int first_quarter = width * 0.25;
	int second_quarter = width * 0.5;
	int third_quarter = width * 0.75;
	int alpha = 0;
	int j_pos = width_offset;

	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha. The alpha channel configures the
		opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_toggle. With this option turned on, this example will display each colour bar
		starting with a 0 alhpa increasing every 3 pixels.
	*/ 
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			
			// reset alpha for each colour bar
			if (j_pos % first_quarter == 0) {
				alpha = 0;
			}
			
			if (j_pos < first_quarter)
			{
				*(frame_buffer_addr++) = 0xff; 
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = alpha;
			}
			else if (j_pos < second_quarter)
			{
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = 0xff;
				*(frame_buffer_addr++) = 0x00;
				*(frame_buffer_addr++) = alpha;
			}
			else if (j_pos < third_quarter)
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
			
			j_pos++;
			if (j_pos == width)
			{
				j_pos = 0;
			}
		}
	}

	width_offset+=2;
	if (width_offset == width) {
		width_offset = 0;
	}

	if (ctx_ld_enable == 1) {
		microkit_notify(52);
	}
}










// It could save the settings for drawing the frame buffer in a struct, then those same settings could be saved and used to clear the frame buffer where it was written at.
void clear_frame_buffer(int width, int height) { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled.
	
	printf("clearing buffer\n");
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
		}
	}
}



// Api example 1
