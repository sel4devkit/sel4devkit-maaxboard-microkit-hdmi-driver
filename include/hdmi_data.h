#ifndef __HDMI_DATA_H__
#define __HDMI_DATA_H__



// Write info linking to spec about these bits set and the extra ones in this memory register. 
// 		     A R G B	
// // ARGB - 3,2,1,0	11100100 E4203
// // AGRB - 3,1,2,0	11011000 D8203
// // ABGR - 3,0,1,2	11000110 C6203
// // AGBR - 3,0,2,1	11001001 C9203
// // ABRG - 3,1,0,2	11010010 D2203
// // ARBG - 3,2,0,1	11100001 E1203
enum RGB_FORMAT {
	RGBA = 0xc6203,
	RBGA = 0xc9203,
	GRBA = 0xd2203,
	GBRA = 0xe1203,
	BGRA = 0xe4203,
	BRGA = 0xd8203
};

struct hdmi_data { // rename 

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
};



// 		  A R G B	
// // ARGB - 3,2,1,0	11100100 E4203
// // AGRB - 3,1,2,0	11011000 D8203
// // ABGR - 3,0,1,2	11000110 C6203
// // AGBR - 3,0,2,1	11001001 C9203
// // ABRG - 3,1,0,2	11010010 D2203
// // ARBG - 3,2,0,1	11100001 E1203



#endif