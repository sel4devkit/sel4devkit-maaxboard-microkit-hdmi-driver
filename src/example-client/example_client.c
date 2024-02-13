#include <microkit.h>
#include <stdio.h>
#include <stddef.h>
#include <vic_table.h>

#define VIC_MODE 2 // Understand different vic modes

uintptr_t frame_buffer;

// tmp declaration - create header in future 
void write_sample_frame_buffer();
void write_test_sample_frame_buffer();
void write_1920_test_frame_buffer();

void init(void) {
	printf("INIT CLIENT UPDATE\n");
    // write_sample_frame_buffer();
	//write_test_sample_frame_buffer();
	write_1920_test_frame_buffer();
}

void
notified(microkit_channel ch) {
}

void write_sample_frame_buffer() { // 1280

	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer;

	for (int i = 0; i < 720; i++) {
		for (int j = 0; j < 1280; j++) { // provide configuration for the pixel resolution - this is based off the current configurations, these values will need to be configurable.
			
			if (j < 320)                                // Provide different configurations for the bit depth - experiment with the different sizes
			{
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 640)
			{
				*(frame_buffer_addr++) = 0x00; // Blue
				*(frame_buffer_addr++) = 0xff; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 960)
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
void write_test_sample_frame_buffer() { // 720
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer;
	for (int i = 0; i < 480; i++) {
		for (int j = 0; j < 720; j++) {

			if (j < 180)                                // Provide different configurations for the bit depth - experiment with the different sizes
			{
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 360)
			{
				*(frame_buffer_addr++) = 0x00; // Blue
				*(frame_buffer_addr++) = 0xff; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 540)
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

void write_1920_test_frame_buffer() {
	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer;
	for (int i = 0; i < 1080; i++) {
		for (int j = 0; j < 1920; j++) {
			if (j < 480)                                // Provide different configurations for the bit depth - experiment with the different sizes
			{
				*(frame_buffer_addr++) = 0xff; // Blue
				*(frame_buffer_addr++) = 0x00; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 960)
			{
				*(frame_buffer_addr++) = 0x00; // Blue
				*(frame_buffer_addr++) = 0xff; // Green
				*(frame_buffer_addr++) = 0x00; // Red
				*(frame_buffer_addr++) = 0x00; // Alpha
			}
			else if (j < 1440)
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