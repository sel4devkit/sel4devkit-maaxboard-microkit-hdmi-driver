#include "dtg.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>


void write_dtg_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {


	if (hdmi_config->alpha_enable == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005484); // need to use shifting instead.
	}
	else {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);
	}
	
	write_register((uint32_t*)(dcss_base + TC_DTG_REG1),
		    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
			hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_CH1_REG4),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + TC_CH1_REG5),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	if (hdmi_config->alpha_enable == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584);	// not sure in the purpose of turning it off - i dont think thats necessary
	}
	else {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); 
	}
}


void write_dtg_memory_registers_ctx_ld(uintptr_t dcss_base, struct hdmi_data *hdmi_config){
	
	printf("ctxld dtg\n");

//	write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);

	write_register((uint32_t*)(dcss_base + TC_DTG_REG1),
		    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
			hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_CH1_REG4),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + TC_CH1_REG5),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
		       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); // this can be configured using the ctrl status bits

	// uint32_t* ctrl_status = (uint32_t*)(dcss_base + TC_CONTROL_STATUS);

	// *ctrl_status |= ((int)1 << 8); 
	// *ctrl_status |= ((int)1 << 8); 
	// *ctrl_status |= ((int)1 << 8); // timing controller enabled
	// *ctrl_status |= ((int)1 << 7); // alpha ch1 pixel

	// *ctrl_status |= ((int)1 << 3);	// rgb enabled
	// *ctrl_status |= ((int)1 << 2); 	// channel_1 enabled
	// // *ctrl_status |= ((int)1 << 1);
	// // *ctrl_status |= ((int)1 << 0);

}