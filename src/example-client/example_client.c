#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>
#include <timer.h>
#include <hdmi_data.h>

uintptr_t frame_buffer;
uintptr_t frame_buffer_start_addr;
uintptr_t timer_base;

struct vic_data *glob_v_data = NULL;

void write_sample_frame_buffer(int width, int height);
void write_static_frame_buffer(int width);
void clear_frame_buffer(int width, int height);
void api_example1(struct vic_mode *v_data);
void vic_table_api_example(int v_mode,struct vic_mode *v_data);
void api_example2(struct vic_mode *v_data);

void init(void) {
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// Store the start of frame buffer
	frame_buffer_start_addr = frame_buffer; // original starting point
	
	// Create structure to hold the vic data
	struct vic_mode *v_data = malloc(sizeof(struct vic_mode));

	// Api examples
	api_example1(v_data);
	api_example2(v_data);

	free(v_data);
}

void api_example1(struct vic_mode *v_data) {

	// Set these values to desired configuration 
	v_data->FRONT_PORCH = 110; // 4
	v_data->BACK_PORCH= 220; // 5
	v_data->HSYNC = 40; // 3
	v_data->TYPE_EOF = 5; // 10
	v_data->SOF = 20; // 11
	v_data->VSYNC= 5; // 9
	v_data->H_ACTIVE = 1280; // 1
	v_data->V_ACTIVE = 720; // 7
	v_data->HSYNC_POL = 1; // 15 (ACTIVE_HIGH)
	v_data->VSYNC_POL = 1; // 16 (ACTIVE HIGH)
	v_data->PIXEL_FREQ_KHZ = 74250; // 13
	v_data->H_BLANK = 370; // 2
	v_data->H_TOTAL = 1650; // 0
	v_data->VIC_R3_0 = 8; // 24
	v_data->VIC_PR = 0; // 25
	v_data->V_TOTAL = 750;

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);
	
	// Display Red, blue green and white collumns on the screen
	write_sample_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
	ms_delay(10000);


	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
	ms_delay(5000);
}


void api_example2(struct vic_mode *v_data) {
	
	// Loop through the first three vic table values to display an image of the same size in three different resolutions
	for (int i = 0; i < 3; i++) {
		vic_table_api_example(i, v_data);
	}
}

void vic_table_api_example(int v_mode,struct vic_mode *v_data) {

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
	
	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);

	// Write a square to buffer at current resolution 
	write_static_frame_buffer(v_data->H_ACTIVE);
	ms_delay(5000);
	
	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
	ms_delay(5000);
}

void clear_frame_buffer(int width, int height) {
	printf("clearing buffer\n");
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	int side_length = 300;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			*(frame_buffer_addr++) = 0x00; // Blue
			*(frame_buffer_addr++) = 0x00; // Green
			*(frame_buffer_addr++) = 0x00; // Red
			*(frame_buffer_addr++) = 0x00; // Alpha
		}
	}
	
}

void
notified(microkit_channel ch) {
}

void write_static_frame_buffer(int width) {
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	int side_length = 300;

	for (int i = 0; i < side_length; i++) {
		for (int j = 0; j < side_length; j++) {
			*(frame_buffer_addr++) = 0xff; // Blue
			*(frame_buffer_addr++) = 0x00; // Green
			*(frame_buffer_addr++) = 0x00; // Red
			*(frame_buffer_addr++) = 0x00; // Alpha
		}
		frame_buffer_addr += 4*(width-side_length);
	}
}

void write_sample_frame_buffer(int width, int height) {
	printf("writing frame buffer...\n");
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	int first_quarter = width * 0.25;
	int second_quarter = width * 0.5;
	int third_quarter = width * 0.75;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			
			if (j < first_quarter)
			{
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < second_quarter)
			{
				*(frame_buffer_addr++) = 0x00; // Blue
				*(frame_buffer_addr++) = 0xff; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < third_quarter)
			{
				*(frame_buffer_addr++) = 0x00; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0xff; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else {
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0xff; // Green
				*(frame_buffer_addr++) = 0xff; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
		}
	}
}