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

struct hdmi_data *hd = NULL;
uintptr_t timer_base;

// Function pointer to current frame buffer function (used for double buffering)
void (*write_fb)(struct hdmi_data*);

void init_api() {
	// Allocate memory to hold the hdmi data
	hd = malloc(sizeof(struct hdmi_data)); // TODO: free at some point

	// Initialise timer
	initialise_and_start_timer(timer_base);
}

void
notified(microkit_channel ch) {
	switch (ch) {
        // Notified by the context loader to draw the frame buffer that is not being displayed
		case 52:								
			start_timer();
			printf("Notified to write buffer\n");
			write_fb(hd);
			printf("Writing frame buffer took %d ms\n", stop_timer());
			microkit_notify(52);
			break;
		default:
			printf("Unexpected channel id: %d in api::notified()\n", ch);
	}
}

void static_image(struct display_config (*init_func)()) {

	// Get the display configurations 
	struct display_config dc = init_func();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_fb(hd);

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

	// Get the display configurations 
	struct display_config dc = init_func();
	*hd = dc.hd;
	
	// Prewrite the buffer
	dc.write_fb(hd);

	// set frame buffer function 
	write_fb = dc.write_fb;

	// Send the hdmi data to the dcss PD to initialise the DCSS, as this example is double buffered it will call back to the examples implementation of write_fb()
	microkit_ppcall(0, seL4_MessageInfo_new((uint64_t)hd, 1, 0, 0));
}

