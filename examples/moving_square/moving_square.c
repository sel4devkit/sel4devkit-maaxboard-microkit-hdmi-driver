 
/* This work is Crown Copyright NCSC, 2024. */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "moving_square.h"

#include "frame_buffer.h"
#include "api.h"

#include "dma_offsets.h"
#include "vic_table.h"

#define MOVING_SQUARE_SIDE_LENGTH 95
#define RBGA_GREEN 0x00ff0000
#define RBGA_BLUE 0x0000ff00
#define RBGA_RED 0x000000ff
#define RBGA_BLACK 0x00000000

#define LIMIT_NOT_SET -5

enum Direction {

	DOWN_RIGHT = 0,
	DOWN_LEFT = 1,
	UP_RIGHT = 2,
	UP_LEFT = 3
};

struct square_positions {

	int x;
	int y;
	int x_limit;
	int y_limit;
	enum Direction x_dir_change;
	enum Direction y_dir_change;
};

struct square_positions square_pos[4] = {{ 1, 1, LIMIT_NOT_SET, LIMIT_NOT_SET, DOWN_LEFT,  UP_RIGHT}, 	// Down right
										 {-1, 1, -1, 			LIMIT_NOT_SET, DOWN_RIGHT, UP_LEFT}, 	// Down left
										 { 1,-1, LIMIT_NOT_SET, -1, 		   UP_LEFT,    DOWN_RIGHT},	// Up right
										 {-1,-1, -1, -1, 		   UP_RIGHT,   DOWN_LEFT}};				// Up Left

// Set these in the init_example_function()
int y_pos = 300;
int x_pos = 5;
int direction = UP_LEFT;

struct buffer_position {
	int x;
	int y;
};


struct buffer_position previous_buffer_position[2];

void init(void) {
	init_api();
	moving_image(init_example);
}

struct display_config init_example()  {

	int v_mode = 0;

   	struct hdmi_data hd;

	hd.FRONT_PORCH = vic_table[v_mode][FRONT_PORCH];
	hd.BACK_PORCH= vic_table[v_mode][BACK_PORCH];
	hd.HSYNC = vic_table[v_mode][HSYNC];
	hd.TYPE_EOF = vic_table[v_mode][TYPE_EOF];
	hd.SOF = vic_table[v_mode][SOF];
	hd.VSYNC= vic_table[v_mode][VSYNC];
	hd.H_ACTIVE = vic_table[v_mode][H_ACTIVE];
	hd.V_ACTIVE = vic_table[v_mode][V_ACTIVE]; 
	hd.HSYNC_POL = vic_table[v_mode][HSYNC_POL];
	hd.VSYNC_POL = vic_table[v_mode][VSYNC_POL];
	hd.PIXEL_FREQ_KHZ = vic_table[v_mode][PIXEL_FREQ_KHZ];
	hd.H_BLANK = vic_table[v_mode][H_BLANK];
	hd.H_TOTAL = vic_table[v_mode][H_TOTAL];
	hd.VIC_R3_0 = vic_table[v_mode][VIC_R3_0];
	hd.VIC_PR = vic_table[v_mode][VIC_PR];
	hd.V_TOTAL = vic_table[v_mode][V_TOTAL];
	hd.rgb_format = RGBA;
	hd.alpha_enable = ALPHA_OFF;
	hd.mode = MOVING_IMAGE;
	hd.ms_delay = NO_DELAY;

	previous_buffer_position[0].x = x_pos;
	previous_buffer_position[0].y = y_pos;

	int x_limit = hd.H_ACTIVE - MOVING_SQUARE_SIDE_LENGTH;
	int y_limit = hd.V_ACTIVE - MOVING_SQUARE_SIDE_LENGTH;
	square_pos[0].x_limit = x_limit;
	square_pos[0].y_limit = y_limit;
	square_pos[1].y_limit = y_limit;
	square_pos[2].x_limit = x_limit;

    // Return struct containing the hdmi data and the function to write the frame buffer
	struct display_config dc = {hd, &write_frame_buffer};
	return dc;
}

int current_fb = 0;

void write_frame_buffer(struct hdmi_data* hd) {
	
	if (hd == NULL){
		printf("hdmi data not yet set, cannot write frame buffer.\n;");
		return;
	}

	int width = hd->H_ACTIVE;
	uint32_t* frame_buffer_addr = get_active_frame_buffer_uint32();
	
	frame_buffer_addr += (width*(previous_buffer_position[current_fb].y)) + previous_buffer_position[current_fb].x;

	// clear the buffer
	for (int i = 0; i < MOVING_SQUARE_SIDE_LENGTH; i++) {
		for (int j = 0; j < MOVING_SQUARE_SIDE_LENGTH; j++) {
			*(frame_buffer_addr++) = RBGA_BLACK;
		}
		frame_buffer_addr += (width-MOVING_SQUARE_SIDE_LENGTH);
	}

	frame_buffer_addr = get_active_frame_buffer_uint32();
	frame_buffer_addr += (width*y_pos) + x_pos;

	for (int i = 0; i < MOVING_SQUARE_SIDE_LENGTH; i++) {
		for (int j = 0; j < MOVING_SQUARE_SIDE_LENGTH; j++) {
			*(frame_buffer_addr++) = RBGA_GREEN;
		}
		frame_buffer_addr += (width-MOVING_SQUARE_SIDE_LENGTH);
	}

	// So the clear function can clear the previously drawn buffer
	previous_buffer_position[current_fb].x = x_pos;
	previous_buffer_position[current_fb].y = y_pos;
	current_fb = current_fb == 0 ? 1 : 0;

	if (y_pos + square_pos[direction].y == square_pos[direction].y_limit) {
		direction = square_pos[direction].y_dir_change;
	}
	else if (x_pos + square_pos[direction].x == square_pos[direction].x_limit) {
		direction = square_pos[direction].x_dir_change;
	}

	y_pos+= square_pos[direction].y;
	x_pos+= square_pos[direction].x;
}