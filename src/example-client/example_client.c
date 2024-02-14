#include <microkit.h>
#include <stdio.h>
#include <stddef.h>
#include <vic_table.h>

#define VIC_MODE  2 // this will be configurable and passed through the notified channel for the specific vic mode

uintptr_t frame_buffer;

void write_sample_frame_buffer(int width, int height);

void init(void) {
	printf("INIT CLIENT UPDATE\n");
	int width;
	int height;

	#if VIC_MODE == 0
		width = 720;
		height = 480;
	#endif
	#if VIC_MODE == 1
		width = 1280;
		height = 720;
	#endif
	#if VIC_MODE == 2
		width = 1920;
		height = 1080;
	#endif

	write_sample_frame_buffer(width, height);
}

void
notified(microkit_channel ch) {
}

void write_sample_frame_buffer(int width, int height) {
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer;

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