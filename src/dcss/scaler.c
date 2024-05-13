
#include "scaler.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>



void write_scaler_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {

	
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_DATA_CTRL), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_FORMAT), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_DST_FORMAT), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_LUMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_CHROMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + 0x1c020),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_DST_CHROMA_RES),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_V_LUMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_LUMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_V_CHROMA_INC), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_CHROMA_INC), 0x00002000);

	write_register((uint32_t*)(dcss_base  + 0x1c0c0), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c140), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + 0x1c180), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c1c0), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + 0x1c000), 0x00000011);


	
	uint32_t* scale_ctrl = (uint32_t*)(dcss_base + SCALE_CTRL);
	*scale_ctrl |= ((int)1 << 0);	// run enable
	*scale_ctrl |= ((int)1 << 4); // sh






	write_register((uint32_t*)(dcss_base  + SCALE_SRC_DATA_CTRL + 0x400), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_FORMAT + 0x400 ), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_DST_FORMAT + 0x400), 0x00000002); // Sets to RGB
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_LUMA_RES + 0x400),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_SRC_CHROMA_RES + 0x400),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + 0x1c020 + 0x400),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_DST_CHROMA_RES + 0x400),
		    ((hdmi_config->V_ACTIVE - 1) << 16 | (hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base  + SCALE_V_LUMA_INC + 0x400), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_LUMA_INC + 0x400), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_V_CHROMA_INC + 0x400), 0x00002000);
	write_register((uint32_t*)(dcss_base  + SCALE_H_CHROMA_INC + 0x400), 0x00002000);

	write_register((uint32_t*)(dcss_base  + 0x1c0c0 + 0x400), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c140 + 0x400), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + 0x1c180 + 0x400), 0x00040000);
	write_register((uint32_t*)(dcss_base  + 0x1c1c0 + 0x400), 0x00000000); // This must stay!
	write_register((uint32_t*)(dcss_base  + 0x1c000 + 0x400), 0x00000011);


	
	uint32_t* scale_ctrl_2 = (uint32_t*)(dcss_base + SCALE_CTRL  + 0x400);
	*scale_ctrl_2 |= ((int)1 << 0);	// run enable
	*scale_ctrl_2 |= ((int)1 << 4); // sh




	// the below appear to have no effect in the current configuration and will likely be removed

	// write_register((uint32_t*)(dcss_base + 0x1c028), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c02c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c030), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c034), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c038), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c03c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c040), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c044), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c048), 0x00000000); //
	// write_register((uint32_t*)(dcss_base + 0x1c04c), 0x00002000);
	// write_register((uint32_t*)(dcss_base + 0x1c050), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c054), 0x00002000);
	// write_register((uint32_t*)(dcss_base + 0x1c058), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c05c), 0x00002000);
	// write_register((uint32_t*)(dcss_base + 0x1c060), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c064), 0x00002000);
	// write_register((uint32_t*)(dcss_base + 0x1c080), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0c0), 0x00040000);
	// write_register((uint32_t*)(dcss_base + 0x1c100), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c084), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0c4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c104), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c088), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0c8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c108), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c08c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0cc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c10c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c090), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0d0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c110), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c094), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0d4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c114), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c098), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0d8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c118), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c09c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0dc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c11c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0a0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0e0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c120), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0a4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0e4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c124), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0a8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0e8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c128), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0ac), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0ec), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c12c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0b0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0f0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c130), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0b4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0f4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c134), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0b8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0f8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c138), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0bc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c0fc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c13c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c140), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c180), 0x00040000); //
	// write_register((uint32_t*)(dcss_base + 0x1c1c0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c144), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c184), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1c4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c148), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c188), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1c8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c14c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c18c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1cc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c150), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c190), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1d0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c154), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c194), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1d4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c158), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c198), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1d8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c15c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c19c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1dc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c160), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1a0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1e0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c164), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1a4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1e4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c168), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1a8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1e8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c16c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1ac), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1ec), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c170), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1b0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1f0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c174), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1b4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1f4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c178), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1b8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1f8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c17c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1bc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c1fc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c300), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c340), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c380), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c304), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c344), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c384), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c308), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c348), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c388), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c30c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c34c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c38c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c310), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c350), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c390), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c314), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c354), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c394), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c318), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c358), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c398), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c31c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c35c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c39c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c320), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c360), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3a0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c324), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c364), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3a4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c328), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c368), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3a8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c32c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c36c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3ac), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c330), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c370), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3b0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c334), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c374), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3b4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c338), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c378), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3b8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c33c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c37c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c3bc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c200), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c240), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c280), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c204), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c244), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c284), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c208), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c248), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c288), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c20c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c24c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c28c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c210), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c250), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c290), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c214), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c254), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c294), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c218), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c258), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c298), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c21c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c25c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c29c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c220), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c260), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2a0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c224), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c264), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2a4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c228), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c268), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2a8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c22c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c26c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2ac), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c230), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c270), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2b0), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c234), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c274), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2b4), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c238), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c278), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2b8), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c23c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c27c), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2bc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c2bc), 0x00000000);
	// write_register((uint32_t*)(dcss_base + 0x1c000), 0x00000011);
}