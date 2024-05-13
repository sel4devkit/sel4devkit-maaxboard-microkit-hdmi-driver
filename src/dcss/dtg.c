#include "dtg.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>


void write_dtg_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {


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

	uint32_t* ctrl_status = (uint32_t*)(dcss_base + TC_CONTROL_STATUS);

	*ctrl_status |= ((int)1 << 24); // default alpha
	*ctrl_status |= ((int)1 << 25); 
	*ctrl_status |= ((int)1 << 26); 
	*ctrl_status |= ((int)1 << 27); 
	*ctrl_status |= ((int)1 << 28); 
	*ctrl_status |= ((int)1 << 29); 
	*ctrl_status |= ((int)1 << 30); 
	*ctrl_status |= ((int)1 << 31); 
		
	*ctrl_status |= ((int)1 << 14); // pixel component ordering into the chroma subsampler (CSS) block
	*ctrl_status |= ((int)1 << 12); 
	
	
	*ctrl_status |= ((int)1 << 8); // timing controller enabled
	*ctrl_status |= ((int)1 << 2); 	// channel_1 enabled

	if (hdmi_config->alpha_enable == ALPHA_ON) {
		*ctrl_status |= ((int)1 << 7); // alpha ch1 pixel
	}
}

void write_dtg_memory_registers_ctx_ld(uintptr_t dcss_base, struct hdmi_data *hdmi_config){
	
	printf("ctxld dtg\n");

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


	int	dis_lrc_y = hdmi_config->VSYNC + hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->V_ACTIVE- 1;

	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	uint32_t* ctrl_status = (uint32_t*)(dcss_base + TC_CONTROL_STATUS);

	*ctrl_status |= ((int)1 << 24); // default alpha
	*ctrl_status |= ((int)1 << 25); 
	*ctrl_status |= ((int)1 << 26); 
	*ctrl_status |= ((int)1 << 27); 
	*ctrl_status |= ((int)1 << 28); 
	*ctrl_status |= ((int)1 << 29); 
	*ctrl_status |= ((int)1 << 30); 
	*ctrl_status |= ((int)1 << 31); 
		
	*ctrl_status |= ((int)1 << 14); // pixel component ordering into the chroma subsampler (CSS) block
	*ctrl_status |= ((int)1 << 12); 
	
	*ctrl_status |= ((int)1 << 7); // alpha ch1 pixel
	*ctrl_status |= ((int)1 << 8); // timing controller enabled
	*ctrl_status |= ((int)1 << 2); 	// channel_1 enabled
}
