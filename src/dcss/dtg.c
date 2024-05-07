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

	// write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);
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
	
	// Modify this if not working. Only should bother once i actually see the interupt though. 
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
	
	// //*ctrl_status |= ((int)1 << 10); // enables per pixel apha for chanel 1 
	*ctrl_status |= ((int)1 << 7); // alpha ch1 pixel
	//*ctrl_status |= ((int)1 << 8); // timing controller enabled
	// *ctrl_status |= ((int)1 << 3);	// rgb enabled
	*ctrl_status |= ((int)1 << 2); 	// channel_1 enabled

	// uint32_t* ctx_ld_reg = (uint32_t*)(dcss_base + TC_CTX_LD_REG10);

	// // Double buffered register = 10
	// *ctx_ld_reg |= ((int)1 << 1); 
	// *ctx_ld_reg |= ((int)1 << 3); 

	// // Single buffered register = Default setting 11
	// *ctx_ld_reg |= ((int)1 << 16); 
	// *ctx_ld_reg |= ((int)1 << 17); 
	// *ctx_ld_reg |= ((int)1 << 19); 

	// 480
	// *ctx_ld_reg |= ((int)1 << 23); 
	// *ctx_ld_reg |= ((int)1 << 26); 


	// The coorinates in display trave for when Line 1 interupt will be triggered.
	// This will be used to "kick" the context loader
	int	dis_lrc_y = hdmi_config->VSYNC + hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->V_ACTIVE- 1;
	write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13),  ((90 * dis_lrc_y) / 100) << 16);
	write_register((uint32_t*)(dcss_base + TC_LINE2_INT_REG14), 0);





	uint32_t* intr_status = (uint32_t*)(dcss_base + TC_INTERRUPT_STATUS);
	int intrstatus_0_enabled = (*intr_status >> 0) & (int)1;
	printf("status before enable = %d\n", intrstatus_0_enabled);


	// Setting the interupts to be enabled (both line 0 and 1 because its unclear which one is used)
	// These may need to be set each time the interrupt has happened. 
	uint32_t* intr_mask = (uint32_t*)(dcss_base + TC_INTERRUPT_MASK);

	int intr_0_enabled = (*intr_mask >> 0) & (int)1;
	intr_0_enabled = (*intr_mask >> 0) & (int)1;
	printf("Enabled before = %d\n", intr_0_enabled);

	*ctrl_status |= ((int)1 << 8); // timing controller enabled
	*intr_mask |= ((int)1 << 0); // line 0 
	*intr_mask |= ((int)1 << 1); // line 1
	*intr_mask |= ((int)1 << 2); // line 1
	*intr_mask |= ((int)1 << 3); // line 1


	intr_0_enabled = (*intr_mask >> 0) & (int)1;
	printf("Enabled after = %d\n", intr_0_enabled);


	intrstatus_0_enabled = (*intr_status >> 0) & (int)1;
	printf("status after enable = %d\n", intrstatus_0_enabled);
	
	// *intr_mask |= ((int)1 << 0); // line 0 
	// intrstatus_0_enabled = (*intr_status >> 0) & (int)1;
	// printf("status again= %d\n", intrstatus_0_enabled);

	*ctrl_status |= ((int)1 << 8); // timing controller enabled

}





void write_dtg_memory_registers_two_channel(uintptr_t dcss_base, struct hdmi_data *hdmi_config){

	// write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);
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
	
	// Proably not needed unless im doing context switching... 
	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);


	uint32_t* ctrl_status = (uint32_t*)(dcss_base + TC_CONTROL_STATUS);

	// *ctrl_status |= ((int)1 << 24); // default alpha
	// *ctrl_status |= ((int)1 << 25); 
	// *ctrl_status |= ((int)1 << 26); 
	// *ctrl_status |= ((int)1 << 27); 
	// *ctrl_status |= ((int)1 << 28); 
	// *ctrl_status |= ((int)1 << 29); 
	// *ctrl_status |= ((int)1 << 30); 
	// *ctrl_status |= ((int)1 << 31);
		
	*ctrl_status |= ((int)1 << 14); // pixel component ordering into the chroma subsampler (CSS) block
	*ctrl_status |= ((int)1 << 12); 
	
	*ctrl_status |= ((int)1 << 10); // enables per pixel apha for chanel 1 
	*ctrl_status |= ((int)1 << 7); // alpha ch1 pixel
	// *ctrl_status |= ((int)1 << 9); // timing controller enabled
	*ctrl_status |= ((int)1 << 8); // timing controller enabled
	*ctrl_status |= ((int)1 << 3);	// rgb enabled
	*ctrl_status |= ((int)1 << 2); 	// channel 1 enabled
	*ctrl_status |= ((int)1 << 1); 	// channel 2 enabled
	*ctrl_status |= ((int)1 << 0); 	// channel 3 enabled

	*ctrl_status |= ((int)1 << 8); // timing controller enabled

}





void check_irq(uintptr_t dcss_base){
	uint32_t* intr_status = (uint32_t*)(dcss_base + TC_INTERRUPT_STATUS);
	int intrstatus_0_enabled = (*intr_status >> 0) & (int)1;
	printf("status after delay = %d\n", intrstatus_0_enabled);
}