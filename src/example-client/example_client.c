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
uintptr_t timer_base;

struct hdmi_data *hdmi_config = NULL;

// this could be wrappe up in a structure of some sort so its clear it belongs to the api_3 example
int width_offset = 0;
int ctx_ld_enable = 0;

void api_example1();
void api_example2();
void api_example3();
void clear_frame_buffer();
void vic_table_api_example(int v_mode);
void write_api_example_1_frame_buffer();
void write_api_example_3_frame_buffer();
void write_api_example_2_frame_buffer();

void init(void) {
	
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// Allocate memory to hold the vic data
	hdmi_config = malloc(sizeof(struct hdmi_data));

	//api_example1(); 	// Display 4 colour bars RGB and white split evenly across the screen with a custom configuration.
	//api_example2();		// Display a square with the same number of pixels at three different resolutions
	api_example3();		// Display 4 coloiur bars RGB and white split evenly across the screen, moving a number of pixels across the screen

	//free(hdmi_config); (This will need to be freed at some point)
	printf("Finished Init Client \n");
}


void
notified(microkit_channel ch) {

	switch (ch) {
        case 52:								// Notified by the context loader to draw the frame buffer that is not being displayed
			//write_api_example_3_frame_buffer(); // Put in a mechanism so that there is a global function pointer at the correct function.
			start_timer();
			write_api_example_3_frame_buffer();
			printf("Writing frame buffer took %d ms\n", stop_timer());
			break;
		default:
			printf("Unexpected channel id: %d in example_client::notified() \n", ch);
	}
}

void api_example1() {
	
	// Initialise the hdmi data with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_ON, DB_OFF};
	*hdmi_config = v;

	// prewrite the buffer before it is displayed
	write_api_example_1_frame_buffer();

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0)); // This funciton is no longer needed as the hdmi_config can just be passed straight in.

	// how long the example is shown for
	ms_delay(10000);

	// Clear the frame buffer
	clear_frame_buffer();

	// Reset the DCSS for next example to avoid strange visual effects caused by prewriting the buffer.
	microkit_notify(55); 
}

void api_example2() {
	
	// Loop through the first three vic table values to display an image of the same size in three different resolutions
	for (int i = 0; i < 3; i++) {
		vic_table_api_example(i);
	}
}

void vic_table_api_example(int v_mode) {

	// Initialise the data from the predefined vic_table
	hdmi_config->FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
	hdmi_config->BACK_PORCH= vic_table[v_mode][BACK_PORCH];
	hdmi_config->HSYNC = vic_table[v_mode][HSYNC];
	hdmi_config->TYPE_EOF = vic_table[v_mode][TYPE_EOF];
	hdmi_config->SOF = vic_table[v_mode][SOF];
	hdmi_config->VSYNC= vic_table[v_mode][VSYNC];
	hdmi_config->H_ACTIVE = vic_table[v_mode][H_ACTIVE];
	hdmi_config->V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
	hdmi_config->HSYNC_POL = vic_table[v_mode][HSYNC_POL];
	hdmi_config->VSYNC_POL = vic_table[v_mode][VSYNC_POL];
	hdmi_config->PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
	hdmi_config->H_BLANK = vic_table[v_mode][H_BLANK];
	hdmi_config->H_TOTAL = vic_table[v_mode][H_TOTAL];
	hdmi_config->VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
	hdmi_config->VIC_PR = vic_table[v_mode][VIC_PR];
	hdmi_config->V_TOTAL = vic_table[v_mode][V_TOTAL];
	hdmi_config->rgb_format = RBGA;
	hdmi_config->alpha_toggle = ALPHA_OFF;
	hdmi_config->db_toggle = DB_OFF;

	// Write a square of a fixed size to the frame buffer at current resolution 
	write_api_example_2_frame_buffer();

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));

	// how long the example is shown for
	ms_delay(10000);
	
	// Clear the frame buffer
	clear_frame_buffer();

	// Reset the DCSS for next example to avoid strange visual effects caused by prewriting the buffer.
	microkit_notify(55); 
}


void api_example3() {

	// Initialise the vic mode with custom values
	struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_OFF, DB_ON};
	*hdmi_config = v;

	// pre write the buffer before display configuration (This would probably be done in a separate PD?)
	write_api_example_3_frame_buffer(0);

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hdmi_config, 1, 0, 0));
	
}

void write_api_example_1_frame_buffer() {
	
	//printf("writing frame buffer...\n");
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

	if (hdmi_config == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	int height = hdmi_config->V_ACTIVE;
	int width = hdmi_config->H_ACTIVE;

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

void write_api_example_2_frame_buffer() {
	
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

	if (hdmi_config == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	int width = hdmi_config->H_ACTIVE;
	
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

void write_api_example_3_frame_buffer() {
	
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

	uint8_t* frame_buffer_start_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);
	printf("frame buffer addr before write = %p\n", frame_buffer_addr);

	if (hdmi_config == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	int height = hdmi_config->V_ACTIVE;
	int width = hdmi_config->H_ACTIVE;

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

	//seL4_ARM_VSpace_Invalidate_Data(3, (long)frame_buffer_start_addr, (long)frame_buffer_addr);
	//int ret = seL4_ARM_VSpace_CleanInvalidate_Data(2, 0, (long)CTX_LD_DMA_SIZE);
	//printf("Return from cache flush = %d\n", ret);
	printf("frame buffer start addr = %p\n", frame_buffer_start_addr);
	printf("frame buffer addr after write = %p\n", frame_buffer_addr);
	//ms_delay(5000);

	// The first time this is called it is pre writing the buffer before any DCSS call, so it should not call the context loader as it will have not yet been initialised.
	if (ctx_ld_enable == 1) {
		microkit_notify(52);	// Call the context loader to signify the buffer has finished being written to.
	}
	else {
		ctx_ld_enable = 1;
	}
}

// It could save the settings for drawing the frame buffer in a struct, then those same settings could be saved and used to clear the frame buffer where it was written at.
void clear_frame_buffer() { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled.
	
	printf("clearing buffer\n");
	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);
	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);

	if (hdmi_config == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}
	
	int height = hdmi_config->V_ACTIVE;
	int width = hdmi_config->H_ACTIVE;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
			*(frame_buffer_addr++) = 0x00;
		}
	}
}

