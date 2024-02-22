#include <microkit.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <vic_table.h>
#include <timer.h>
#include <hdmi_data.h>

uintptr_t frame_buffer;
uintptr_t frame_buffer_start_addr;
uintptr_t timer_base;

int dimensions[3][2] = {{720, 480}, {1280, 720}, {1920, 1080}};

void write_sample_frame_buffer(int width, int height);
void write_test_frame_buffer(int width, int height);
void write_static_frame_buffer(int width);
void clear_frame_buffer(int width, int height);

void init(void) {
	printf("INIT CLIENT UPDATE\n");

	initialise_and_start_timer(timer_base);
	
	int vic_mode;
	frame_buffer_start_addr = frame_buffer; // original starting point


	//struct vic_data *v_data = malloc(sizeof(struct vic_data));
	// v_data->a = 100;
	// v_data->b = 200;


	// microkit_ppcall(0, seL4_MessageInfo_new((uint64_t) v_data,1,0,0));


	for (int i = 0; i < 3; i++) {
		printf("testing timer... %d\n", i);
		vic_mode = i;
		microkit_notify(46);
		// ms_delay(1000);
		//write_sample_frame_buffer(dimensions[vic_mode][0], dimensions[vic_mode][1]);
		write_static_frame_buffer(dimensions[vic_mode][0]); // currently writes a square 
		ms_delay(5000);
		clear_frame_buffer(dimensions[vic_mode][0], dimensions[vic_mode][1]); // These will be got from the vic_data struct, instead of the dimensions array. 
		ms_delay(5000);
	}
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

void write_test_frame_buffer(int width, int height) {

	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer;
	//for (int i = 0; i < height/2; i++) {
		for (int j = 0; j < width; j++) {
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
		}
	//}
}