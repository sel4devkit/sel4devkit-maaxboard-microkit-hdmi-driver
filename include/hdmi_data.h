#ifndef __HDMI_DATA_H__
#define __HDMI_DATA_H__


struct vic_data {
    int a;
    int b;
};


struct vic_mode {

    int H_TOTAL;
	int H_ACTIVE;
	int H_BLANK;
	int HSYNC;
	int FRONT_PORCH;
	int BACK_PORCH;
	int V_TOTAL;
	int V_ACTIVE;
	int V_BLANK;
	int VSYNC;
	int TYPE_EOF;
	int SOF;
	int V_FREQ_HZ;
	int PIXEL_FREQ_KHZ;
	int I_P;
	int HSYNC_POL;
	int VSYNC_POL;
	int START_OF_F0;
	int START_OF_F1;
	int VSYNC_START_INTERLACED_F0;
	int VSYNC_END_INTERLACED_F0;
	int VSYNC_START_INTERLACED_F1;
	int VSYNC_END_INTERLACED_F1;
	int VIC;
	int VIC_R3_0;
	int VIC_PR;
};



// 1280 * 720
	// {1650, // 0
	// 1280, // 1
	// 370, // 2
	// 40, // 3
	// 110, // 4
	// 220, // 5
	// 750, // 6
	// 720, // 7
	// 30, // 8
	// 5, // 9
	// 5, // 10
	// 20, // 11
	// 60, // 12
	// 74250, // 13
	// PROGRESSIVE, // 14 
	// ACTIVE_HIGH, // 15
	// ACTIVE_HIGH,  // 16
	// 1,  // 17
	// 65535,  // 18
	// 1,  // 19
	// 31,  // 20
	// 65535,  // 21
	// 65535, // 22
	// 4, // 23
	// 8, // 24
	// 0}, // 25

#endif