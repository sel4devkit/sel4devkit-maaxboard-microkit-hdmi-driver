#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>

#include "timer.h"
#include "hdmi_data.h"

#include "dma_offsets.h"


// enum API_EXAMPLE {
// 	API_EXAMPLE_ONE  = 1,
// 	API_EXAMPLE_TWO  = 2,
// 	API_EXAMPLE_THREE = 3
// };

uintptr_t frame_buffer; // This should be called the DMA pool

// An array containing two pointers should be set up to point to each frame buffer.
// The frame_buffer_start_addr will point to either one of these. 
// In a staic example the fbsa pointer will never be reset.
// In a double buffered example, the fbsa pointer will change to point to either one in the array when it is notified by the dcss through a channel.
// On each new notification it will swap the index using a tenary operator.

uintptr_t frame_buffers[2];
uintptr_t frame_buffer_start_addr;
uintptr_t timer_base;

struct hdmi_data *v_data = NULL;

void api_example1();
// void api_example2(struct hdmi_data *v_data);
void api_example3();
void clear_frame_buffer(int width, int height);
// void vic_table_api_example(int v_mode,struct hdmi_data *v_data);
void write_api_example_1_frame_buffer(int width, int height);
void write_api_example_3_frame_buffer(int width, int height);
// void write_static_frame_buffer(int width);
// void write_moveable_frame_buffer(int width, int height, int offset);

int current_buffer_index = 0;

void init(void) {
	
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// In the future I could abstract the frame buffer code away from the client.
	// A way to do this would be to pass the address of the frame buffer via a protected procedure.
	frame_buffers[0] = frame_buffer; // change name to dma pool
	frame_buffers[1] = frame_buffer + FRAME_BUFFER_TWO_OFFSET; 

	
	// Store the start of frame buffer (This will change for moveable frame buffers - perhaps this should be retrieved through a notification)
	// This frame buffer will just be one frame buffer, if a moving image then where this points will change, but the user will jsut use the same one.
	frame_buffer_start_addr = frame_buffers[0];
	
	// Allocate memory to hold the vic data
	v_data = malloc(sizeof(struct hdmi_data));

	// Currently these are commented out depending on which one is needed
	//api_example1(); // Display 4 colour bars RGB and white split evenly across the screen with a custom configuration.

	api_example3();

	// For the rotating colour bars, it would simply be the same loop but it would start the position of the buffer at a different point
	// Once the position of the frame buffer reaches the end (which would be already defined), then it would reset the frame buffer to the start
	// If its writing the entire screen, then it doesn't need to worry about the clear function. 

	//free(v_data); (This will need to be freed at some point)

	printf("Finished Init Client \n");
}


void
notified(microkit_channel ch) {

	printf("Example client notified called\n");

	switch (ch) {
        case 52:
			// Currently just implemented for the initial double buffered example.

			current_buffer_index = current_buffer_index == 1 ? 0 : 1; // switch the current buffer index
			frame_buffer_start_addr = frame_buffers[current_buffer_index];
			write_api_example_3_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
			break;
		default:
			printf("Unexpected channel id: %d in example_client::notified() \n", ch);
	}


	ms_delay(1000);
	// For double buffered examples it will need to change the frame buffer to the alternate one.
	// A struct can keep track of which example to be running and which frame buffer
}

microkit_msginfo
protected(microkit_channel ch, microkit_msginfo msginfo) {
}

void api_example1() {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_ON, DB_OFF};
	*v_data = v;

	// pre write the buffer before display configuration
	write_api_example_1_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE); // this will be defined in a separate file. The frame buffer ADDR will be psased in

	// Send the vic mode data to the dcss PD
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0)); // This funciton is no longer needed as the v_data can just be passed straight in.
	
	// Send a message to the dcss PD to initialise the DCSS using the vic data
	microkit_notify(46); // This will just need the V data passed in but it will call init_dcss().

	// it will then draw the frame buffer it needs to.

	// if it was using the context loader, then a function pointer would be passed in so that it can use the correct frame buffer function.
	// some params like the frame buffer number (0 or 1) may be needed. 
}

void api_example3() {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_OFF, DB_ON};
	*v_data = v;

	// pre write the buffer before display configuration (This would probably be done in a separate PD?)
	write_api_example_3_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);

	// Send the vic mode data to the dcss PD
	// 
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

	// This timer would ideally be external to this and be used to switch between api examples. A clear would happen at the end (or a reset of the buffer at the start of the next example)
	//ms_delay(15000);
	//clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
}


void write_api_example_3_frame_buffer(int width, int height) {
	
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
	printf("frame buffer addr = %p\n", frame_buffer_addr);

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



















// It could save the settings for drawing the frame buffer in a struct, then those same settings could be saved and used to clear the frame buffer where it was written at.
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


// void api_example2(struct hdmi_data *v_data) {
	
// 	// Loop through the first three vic table values to display an image of the same size in three different resolutions
// 	for (int i = 0; i < 3; i++) {
// 		vic_table_api_example(i, v_data);
// 	}
// }

// void api_example3(struct hdmi_data *v_data) {
	
// }

// void vic_table_api_example(int v_mode,struct hdmi_data *v_data) {

// 	// Initialise the data from the predefined vic_table
// 	v_data->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
// 	v_data->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
// 	v_data->HSYNC = vic_table[v_mode][HSYNC];
// 	v_data->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
// 	v_data->SOF = vic_table[v_mode][SOF];
// 	v_data->VSYNC= vic_table[v_mode][VSYNC];
// 	v_data->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
// 	v_data->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
// 	v_data->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
// 	v_data->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
// 	v_data->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
// 	v_data->H_BLANK = vic_table[v_mode][H_BLANK];
// 	v_data->H_TOTAL = vic_table[v_mode][H_TOTAL];
// 	v_data->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
// 	v_data->VIC_PR = vic_table[v_mode][VIC_PR];
// 	v_data->V_TOTAL = vic_table[v_mode][V_TOTAL];
// 	v_data->rgb_format = RBGA;
// 	v_data->alpha_toggle = ALPHA_OFF;
// 	v_data->db_toggle = DB_OFF;

// 	// Send the vic mode data to the dcss PD
// 	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)v_data, 1, 0, 0));
	
// 	// Send a message to the dcss PD to initialise the DCSS using the vic data
// 	microkit_notify(46);

// 	// Write a square of a fixed size to the frame buffer at current resolution 
// 	write_static_frame_buffer(v_data->H_ACTIVE);
// 	ms_delay(30000);
	
// 	// Clear the frame buffer
// 	clear_frame_buffer(v_data->H_ACTIVE, v_data->V_ACTIVE);
// }

// void write_moveable_frame_buffer(int width, int height, int offset) {

// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
// 	int side_length = height;
// 	int side_width = 20;

// 	// is offset increasing??
// 	printf("Offset = %d\n", offset);
	
// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha.
// 		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle.
// 	*/ 
// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0xff;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0xff;

// 			// could we get a 32bit pointer to store all of these addresses, then when it comes to clearing it just goes through thees?
// 			// initially i can just go for the straight clear
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}
	
// 	ms_delay(100);
// 	frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
// 	// Clear only the pixels that were drawn
// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;

// 			// could we get a 32bit pointer to store all of these addresses, then when it comes to clearing it just goes through thees?
// 			// initially i can just go for the straight clear
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}

// }

// // Api example 2
// void write_static_frame_buffer(int width) {
	
// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer_start_addr;
// 	int side_length = 300;
	
// 	/*
// 		Each of the 4 values written to the frame buffer reprsents a 32 bit RGBA channel.
// 		They are written in the order of the hdmi_data.rgb_format member. If the format is GBRA for example, 
// 		Then the order of the values written below will be green, blue, red, alpha.
// 		The alpha channel configures the opacity of the colour, at 0xff it will be completely visible and 0x00 it will not be visible.
// 		It is turned on or off using hdmi_data.alpha_toggle.
// 	*/ 
// 	for (int i = 0; i < side_length; i++) {
// 		for (int j = 0; j < side_length; j++) {
// 			*(frame_buffer_addr++) = 0xff;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0xff;
// 		}
// 		frame_buffer_addr += 4*(width-side_length);
// 	}
// }

// Api example 1
