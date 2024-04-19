#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <microkit.h>

#include <vic_table.h>

#include "api.h"
#include "timer.h"
#include "hdmi_data.h"
#include "dma_offsets.h"

#include "frame_buffer.h"

#include "api_example_1.h"
#include "api_example_2.h"
#include "api_example_3.h"
#include "api_example_4.h"
#include "empty_client.h"

uintptr_t timer_base;
struct hdmi_data *hd = NULL;

// Function pointer to current frame buffer function (used for double buffering)
void (*write_frame_buffer)(struct hdmi_data*);

void init(void) {
	
	// Initialise timer
	initialise_and_start_timer(timer_base);
	
	// Allocate memory to hold the hdmi data
	hd = malloc(sizeof(struct hdmi_data)); // TODO: free at some point

	// moving_image(init_example_4); // here for testing 

	run_examples(); // This will run indefinitely - might be an idea to put an iteration counter in
	
	/* Write your frame buffer in write_empty_client_static_frame_buffer() or write_empty_client_moving_frame_buffer() */
	
	//static_image(init_empty_client_moving_frame_buffer);
	//moving_image(init_empty_client_moving_frame_buffer);
}

void
notified(microkit_channel ch) {
	switch (ch) {
        // Notified by the context loader to draw the frame buffer that is not being displayed
		case 52:								
			start_timer();
			printf("Notified to write buffer\n");
			write_frame_buffer(hd);
			printf("Writing frame buffer took %d ms\n", stop_timer());
			microkit_notify(52);
			break;
		default:
			printf("Unexpected channel id: %d in api::notified() \n", ch);
	}
}


void static_image(struct display_config (*init_func)()) {

	struct display_config dc = init_func();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_frame_buffer(hd);

	// Send the hdmi data to the dcss PD to initialise the DCSS
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hd, 1, 0, 0));
}

void reset_static_image(int ms) {

	// How long the example is shown for
	ms_delay(ms);
	
	// Clear the frame buffer
	clear_current_frame_buffer(hd);

	// Reset the DCSS for next example
	microkit_notify(55); 
}

void moving_image(struct display_config (*init_func)()){

	struct display_config dc = init_func();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_frame_buffer(hd);

	// set frame buffer function 
	write_frame_buffer = dc.write_frame_buffer;

	// Send the hdmi data to the dcss PD to initialise the DCSS, as this example is double buffered it will call back to the examples implementation of write_frame_buffer()
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hd, 1, 0, 0));
}

void run_examples() {

	static_image(init_example_1);
	reset_static_image(5000);
	
	// show the same sized image at three different resolutions
	for (int i = 0; i < 3; i++) {
		set_example_2_vic_mode(i);
		static_image(init_example_2);
		reset_static_image(5000);
	}
	moving_image(init_example_3);
}


