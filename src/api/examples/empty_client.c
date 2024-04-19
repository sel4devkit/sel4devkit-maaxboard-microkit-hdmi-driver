
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "empty_client.h"
#include "frame_buffer.h"

#include "dma_offsets.h"
#include "vic_table.h"

struct display_config init_empty_client_static_frame_buffer() {

	/* Init hd here with custom values or use pre-configured values from vic_table.h */
    struct hdmi_data hd;


	/* Return struct containing the hdmi data and the function to write the frame buffer */
	struct display_config dc = {hd, &write_empty_client_static_frame_buffer};
	return dc;

}

void write_empty_client_static_frame_buffer(struct hdmi_data* hd){

    if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

    /* Get the frame buffer as a 8, 32 or 64 bit pointer */
	uint32_t* frame_buffer_addr = get_active_frame_buffer_uint32();

    /* The width and height of the active display region */
    int height = hd->V_ACTIVE;
	int width = hd->H_ACTIVE;


    /* Write to the frame buffer to display a static image */
	for (int i = 0; i < height; i++) { 
		for (int j = 0; j < width; j++) {
     
        }
    }
}


struct display_config init_empty_client_moving_frame_buffer(){

    /* Init hd here with custom values or use pre-configured values from vic_table.h */
    struct hdmi_data hd;


	/* Return struct containing the hdmi data and the function to write the frame buffer */
	struct display_config dc = {hd, &write_empty_client_moving_frame_buffer};
	return dc;

}
void write_empty_client_moving_frame_buffer(struct hdmi_data* hd){

    if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

    /* Get the frame buffer as a 8, 32 or 64 bit pointer */
	uint32_t* frame_buffer_addr = get_active_frame_buffer_uint32();


    /* The width and height of the active display region */
    int height = hd->V_ACTIVE;
	int width = hd->H_ACTIVE;


    /* Write to the frame buffer here. This function will be called continously to update the buffer.
       Use global variables to alter the contents of this buffer so that the values persist */
	for (int i = 0; i < height; i++) { 
		for (int j = 0; j < width; j++) {
     
        }
    }
}