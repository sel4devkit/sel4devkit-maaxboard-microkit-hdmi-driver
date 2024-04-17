#ifndef __HDMI_DATA_H__
#define __HDMI_DATA_H__

/*
	These configurations are written to the MODE_CTRL0 memory register in the DPR (15.7.3.1.18).
	The R,G,B and A channels are represented each by 8 bits. Within this memory register values 0,1,2,3 are assigned to each channel
	to determine the order that they will be written to in the buffer. The bits from 12-19 are used to set the RGBA values. The
	enum names the formats are reversed compared to the below table (see last row) as it is written as little endian in the memory register.
	
	ABGR - 3,0,1,2	11000110  RGBA
	AGBR - 3,0,2,1	11001001  RBGA
	ABRG - 3,1,0,2	11010010  GRBA
	ARBG - 3,2,0,1	11100001  GBRA

	ARGB - 3,2,1,0	11100100  BGRA
	AGRB - 3,1,2,0	11011000  BRGA

	In This memory register, other settings for the RTRAM configuration as well as pixel size are set to fixed values. If these need
	to be changed, these rgb formats will need to be changed to accomodate this. Bit masking to toggle different bits would be a better
	approach in this case. 

*/ 
enum RGB_FORMAT {
	RGBA = 0xc6203,
	RBGA = 0xc9203,
	GRBA = 0xd2203,
	GBRA = 0xe1203,
	BGRA = 0xe4203,
	BRGA = 0xd8203
};

/*
	This enum is used to toggle on the alpha channel in the frame buffer. If it is on then a range from 0-256 can be written into
	the buffer to set the RGB colours opacity. If it is set to off, writing to the alpha channel will have no effect. This is
	achieved by selecting certain values to be written in the DTG memory register TC_CONTROL_STATUS. This is acheived by changing 1
	bit in the memory register at position number 10 TC_CH1_PER_PEL_ALPHA_SEL. If more configuration needs to be done to this memory register
	to change other settings then bit masking to toggle the different bits would be a better approach. 

*/
enum ALPHA_TOGGLE {

	ALPHA_OFF,
	ALPHA_ON
};

/*
	This enum is used to toggle double buffering on or off.
*/
enum DOUBLE_BUFFER_TOGGLE {

	DB_OFF,
	DB_ON
};

/*
	This struct holds the user defined display configurations that are set in the client PD and used in the DCSS PD to configure the maaxboard.
	All those that are in capitals are used to set the VIC data. The other fields are used for extra display configurations. 
*/
struct hdmi_data {

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
	enum RGB_FORMAT rgb_format;
	enum ALPHA_TOGGLE alpha_toggle;
	enum DOUBLE_BUFFER_TOGGLE db_toggle;
	int ms_delay;
};



struct display_config {

	struct hdmi_data hd;
	void (*write_frame_buffer)();
};

#define NO_DELAY -1

#endif