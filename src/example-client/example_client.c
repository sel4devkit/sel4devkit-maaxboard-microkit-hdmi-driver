#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>

#include "timer.h"
#include "hdmi_data.h"

uintptr_t frame_buffer;
uintptr_t frame_buffer_start_addr;
uintptr_t timer_base;

struct vic_data *glob_v_data = NULL;

void api_example1(struct hdmi_data *v_data);
void api_example2(struct hdmi_data *v_data);
void api_example3(struct hdmi_data *v_data);
void clear_frame_buffer(int width, int height);
void vic_table_api_example(int v_mode,struct hdmi_data *v_data);
void write_sample_frame_buffer(int width, int height);
void write_static_frame_buffer(int width);
void write_moveable_frame_buffer(int width, int height, int offset);

void init(void) {
	
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// Store the start of frame buffer
	frame_buffer_start_addr = frame_buffer;
	
	// Create structure to hold the vic data
	struct hdmi_data *v_data = malloc(sizeof(struct hdmi_data));

	// Api examples
	//api_example1(v_data); // Display 4 colour bars RGB and white split evenly across the screen with a custom configuration.
	//api_example2(v_data); // Loop through each predefined VIC table values to draw the same sized square at different resolutions
	api_example3(v_data); 

	free(v_data);
}

void api_example1(struct hdmi_data *v_data) {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_ON};
	*v_data = v;

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);
	
	// Display Red, blue green and white collumns on the screen
	write_sample_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
	ms_delay(15000);

	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}

void api_example2(struct hdmi_data *v_data) {
	
	// Loop through the first three vic table values to display an image of the same size in three different resolutions
	for (int i = 0; i < 3; i++) {
		vic_table_api_example(i, v_data);
	}
}

void api_example3(struct hdmi_data *v_data) {
	
	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, RBGA, ALPHA_OFF};
	*v_data = v;

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);
	
	// Display Red, blue green and white collumns on the screen

	for (int offset = 0; offset < 500; offset++) {
		write_moveable_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE, offset);
		// Clear the frame buffer
		// ms_delay(100);
		// clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
		//ms_delay(250);
	}
		
	ms_delay(15000);

	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}

void vic_table_api_example(int v_mode,struct hdmi_data *v_data) {

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

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46);

	// Write a square of a fixed size to the frame buffer at current resolution 
	write_static_frame_buffer(v_data->H_ACTIVE);
	ms_delay(30000);
	
	// Clear the frame buffer
	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}

void clear_frame_buffer(int width, int height) { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled. 
	
	printf("clearing buffer\n");
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;

	int side_length = 300;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
		}
	}
}

void
notified(microkit_channel ch) {
}

void write_moveable_frame_buffer(int width, int height, int offset) {

	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	int side_length = height;
	int side_width = 20;

	// is offset increasing??
	printf("Offset = %d\n", offset);
	
	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha.
		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_toggle.
	*/ 
	for (int i = 0; i < side_length; i++) {
		frame_buffer_addr += 4*(offset);
		for (int j = 0; j < side_width; j++) {
			*(frame_buffer_addr++) = 0xff;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0xff;

			// could we get a 32bit pointer to store all of these addresses, then when it comes to clearing it just goes through thees?
			// initially i can just go for the straight clear
		}
		frame_buffer_addr += 4*(width-side_width-offset);
	}
	
	ms_delay(100);
	frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	// Clear only the pixels that were drawn
	for (int i = 0; i < side_length; i++) {
		frame_buffer_addr += 4*(offset);
		for (int j = 0; j < side_width; j++) {
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;

			// could we get a 32bit pointer to store all of these addresses, then when it comes to clearing it just goes through thees?
			// initially i can just go for the straight clear
		}
		frame_buffer_addr += 4*(width-side_width-offset);
	}

}

void write_static_frame_buffer(int width) {
	
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	int side_length = 300;
	
	/*
		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
		Then the order of the values written below will be green, blue, red, alpha.
		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
		It is turned on or off using hdmi_data.alpha_toggle.
	*/ 
	for (int i = 0; i < side_length; i++) {
		for (int j = 0; j < side_length; j++) {
			*(frame_buffer_addr++) = 0xff;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0xff;
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
	int alpha = 0;

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