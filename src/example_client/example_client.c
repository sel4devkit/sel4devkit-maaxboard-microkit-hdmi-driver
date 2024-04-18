#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>

#include "example_client.h"
#include "timer.h"
#include "hdmi_data.h"
#include "dma_offsets.h"

#include "frame_buffer.h"
#include "api_examples.h"

//#include "api_example_1.h"

#define API_EXAMPLE_2_SIDE_LENGTH 300

#define RGBA_RED 0x000000ff		// It would be better to have these as like positions, 0,1,2 then depending on the format whatever is at 0,1,2 is then itialised with these
#define RGBA_GREEN 0x0000ff00
#define RGBA_BLUE 0x00ff0000
#define RGBA_WHITE 0x00ffffff
#define RGBA_RED_64 0x000000ff000000ff
#define RGBA_GREEN_64 0x0000ff000000ff00
#define RGBA_BLUE_64 0x00ff000000ff0000
#define RGBA_WHITE_64 0x00ffffff00ffffff

// Experimenting chaning values for the alpha channel
#define RGBA_RED_64_5   0x010000ff010000ff
#define RGBA_GREEN_64_5 0x0000ff000000ff00
#define RGBA_BLUE_64_5 0xffff0000ffff0000
#define RGBA_WHITE_64_5 0x00ffffff00ffffff

uintptr_t timer_base;
struct hdmi_data *hd = NULL;

// Function pointer to current frame buffer function (used for double buffering)
void (*write_frame_buffer)(struct hdmi_data*);

void init(void) {
	
	printf("Init Client \n");
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// Allocate memory to hold the vic data
	hd = malloc(sizeof(struct hdmi_data));

	// api_example1();

	api_example_3();

	printf("Finished Init Client \n");
}

void
notified(microkit_channel ch) {
	switch (ch) {
        // Notified by the context loader to draw the frame buffer that is not being displayed
		case 52:								
			start_timer();
			write_frame_buffer(hd);
			printf("Writing frame buffer took %d ms\n", stop_timer());
			microkit_notify(52);
			break;
		default:
			printf("Unexpected channel id: %d in example_client::notified() \n", ch);
	}
}


void api_example_1() {

	struct display_config dc = init_example_1();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_frame_buffer(hd);

	printf("HDMI stuff = %d\n", hd->H_ACTIVE);

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hd, 1, 0, 0));

	// How long the example is shown for
	ms_delay(50000);
	
	// Clear the frame buffer
	clear_current_frame_buffer(hd);

	// Reset the DCSS for next example
	microkit_notify(55); 
}

void api_example_3(){

	struct display_config dc = init_example_3();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_frame_buffer(hd);

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hd, 1, 0, 0));
}

// void write_api_example_1_frame_buffer(struct hdmi_data* hd) { // pass in hdmi config
	
// 	printf("Writing function api 1\n");
	
// 	if (hd == NULL){
// 		printf("hdmi data not yet set, cannot write frame buffer.\n;");
// 		return;
// 	}

// 	//uint8_t* frame_buffer_addr = get_frame_buffer_uint8();


// 	uintptr_t* frame_buffer_addr_offset = (uintptr_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);    // make another define, one for visible buffer and one for the backup.
// 	uint8_t* frame_buffer_addr = (uint8_t*)(dma_base + *frame_buffer_addr_offset);


// 	int height = hd->V_ACTIVE;
// 	int width = hd->H_ACTIVE;
// 	int first_quarter = width * 0.25;
// 	int second_quarter = width * 0.5;
// 	int third_quarter = width * 0.75;
// 	int alpha = 0;


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


