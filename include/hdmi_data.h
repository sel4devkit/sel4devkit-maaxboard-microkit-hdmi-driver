#ifndef __HDMI_DATA_H__
#define __HDMI_DATA_H__

struct vic_mode {

    int H_TOTAL;
	int H_ACTIVE;
	int H_BLANK;
	int HSYNC;
	int FRONT_PORCH;
	int BACK_PORCH;
	int V_TOTAL;
	int V_ACTIVE;
	int VSYNC;
	int TYPE_EOF;
	int SOF;
	int PIXEL_FREQ_KHZ;
	int HSYNC_POL;
	int VSYNC_POL;
	int VIC_R3_0;
	int VIC_PR;
	int VIC;
};

#endif