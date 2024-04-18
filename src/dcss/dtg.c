#include "dtg.h"
#include "write_register.h"

#include <stdio.h>
#include <stdlib.h>


void write_dtg_memory_registers(uintptr_t dcss_base, struct hdmi_data *hdmi_config) {


	if (hdmi_config->alpha_toggle == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005484);
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

	if (hdmi_config->alpha_toggle == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584);
	}
	else {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); 
	}
}


void dtg_context_stuff(){
	// // These are directly from the linux source code, its actually the same as uboot so these don't need to be written like this

	// int dtg_lrc_y = hdmi_config->TYPE_EOF  + hdmi_config->SOF  + hdmi_config->VSYNC + hdmi_config->V_ACTIVE - 1;
	// int dis_ulc_y = hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF- 1;
	// int dis_lrc_y = hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE - 1; // rename 

	// int dtg_lrc_x = hdmi_config->FRONT_PORCH  + hdmi_config->BACK_PORCH  + hdmi_config->HSYNC +
	// 	    hdmi_config->H_ACTIVE - 1;
	// int dis_ulc_x = hdmi_config->HSYNC  + hdmi_config->BACK_PORCH  - 1;
	// int dis_lrc_x = hdmi_config->HSYNC  + hdmi_config->BACK_PORCH  + hdmi_config->H_ACTIVE - 1;


	

	// Kept these with DTG enabled (previous didn't have it enabled, not sure why.)
	// if (hdmi_config->alpha_toggle == ALPHA_ON) {
	// 	write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584); // TODO: add defines
	// }
	// else {
	// 	write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF00518F); 
	// }

	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005486);
	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005586); // with channel 2 and alpha enabled 
	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005786); // with dolby enabled

	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF00058C); // with channel 2 disabled, alpha blend enabled

	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF00050C); // with channel 2 disabled, alpha blend disabled
	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF00010C); // with channel 2 disabled, alpha (11) disabled
	

	// // This one is good for api example 4
	// write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584); // original settings with DTG
	
	// // works with example 5 - alpha things turned on
	// //write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF00558F);

	// //TODO: Tidy these up i should go back to using these
	// write_register((uint32_t*)(dcss_base + TC_DTG_REG1),
	// 	    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
	// 		hdmi_config->H_ACTIVE - 1)));	
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	// write_register((uint32_t*)(dcss_base + TC_CH1_REG4),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	// write_register((uint32_t*)(dcss_base + TC_CH1_REG5),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	
	// 	write_register((uint32_t*)(dcss_base + TC_DTG_REG1),
	// 	    (((hdmi_config->TYPE_EOF+ hdmi_config->TYPE_SOF + hdmi_config->VSYNC + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC +
	// 		hdmi_config->H_ACTIVE - 1)));
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
	// 	    (((hdmi_config->VSYNC + hdmi_config->TYPE_EOF+ hdmi_config->TYPE_SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC + hdmi_config->BACK_PORCH - 1)));
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
	// 	    (((hdmi_config->VSYNC + hdmi_config->TYPE_EOF+ hdmi_config->TYPE_SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC + hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	// write_register((uint32_t*)(dcss_base + TC_CH1_REG4),
	// 	    (((hdmi_config->VSYNC + hdmi_config->TYPE_EOF+ hdmi_config->TYPE_SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC + hdmi_config->BACK_PORCH - 1)));
	// write_register((uint32_t*)(dcss_base + TC_CH1_REG5),
	// 	    (((hdmi_config->VSYNC + hdmi_config->TYPE_EOF+ hdmi_config->TYPE_SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC + hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));
	// write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	// write_register((uint32_t*)(dcss_base + TC_DTG_REG1), ((dtg_lrc_y << 16) | dtg_lrc_x));
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG2), ((dis_ulc_y << 16) | dis_ulc_x));
	// write_register((uint32_t*)(dcss_base + TC_DISPLAY_REG3), ((dis_lrc_y << 16) | dis_lrc_x));

	// swap out these functions for the linux ones... 

	// write_register((uint32_t*)(dcss_base + TC_CH1_REG4),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	
	// write_register((uint32_t*)(dcss_base + TC_CH1_REG5),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));

	// write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);	// original 
	// write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);


		// write_register((uint32_t*)(dcss_base + TC_DTG_REG1),
	// 	    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
	// 		hdmi_config->H_ACTIVE - 1)));	




	// hdmi_info->timings.hfront_porch.typ = vic_table[VIC_MODE][FRONT_PORCH];
	// hdmi_info->timings.hback_porch.typ = vic_table[VIC_MODE][BACK_PORCH];
	// hdmi_info->timings.hsync_len.typ = vic_table[VIC_MODE][HSYNC];
	// hdmi_info->timings.vfront_porch.typ = vic_table[VIC_MODE][TYPE_EOF];
	// hdmi_info->timings.vback_porch.typ = vic_table[VIC_MODE][SOF];
	// hdmi_info->timings.vsync_len.typ = vic_table[VIC_MODE][VSYNC];
	// hdmi_info->timings.hactive.typ = vic_table[VIC_MODE][H_ACTIVE];
	// hdmi_info->timings.vactive.typ = vic_table[VIC_MODE][V_ACTIVE];







	// // USed for channel 2 (alpha)
	// write_register((uint32_t*)(dcss_base + TC_CH2_REG6),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	
	// write_register((uint32_t*)(dcss_base + TC_CH2_REG7),
	// 	    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -
	// 	       1) << 16) | (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));


	// int sb_ctxld_trig = (0 * dis_lrc_y / 100);
	// int db_ctxld_trig = (99 * dis_lrc_y / 100);

	//printf("ctxld trig = %x db = %x\n", sb_ctxld_trig, db_ctxld_trig);

	//ms_delay(100000);
	
	// uint32_t* ctx_ld = (uint32_t*)(dcss_base + TC_CTX_LD_REG10);
	
	// *ctx_ld |= ((int)1 << 16); // these three are for the single buffered registers, but only seem to have an effect.
	// *ctx_ld |= ((int)1 << 17);
	// *ctx_ld |= ((int)1 << 19);

	// // *ctx_ld |= ((int)1 << 12);
	// // *ctx_ld |= ((int)1 << 11);
	// // *ctx_ld |= ((int)1 << 10);
	// // *ctx_ld |= ((int)1 << 9);
	// // *ctx_ld |= ((int)1 << 8);
	// // *ctx_ld |= ((int)1 << 7);
	// // *ctx_ld |= ((int)1 << 6);
	// // *ctx_ld |= ((int)1 << 5);
	// // *ctx_ld |= ((int)1 << 4);
	// *ctx_ld |= ((int)1 << 3);
	// // *ctx_ld |= ((int)1 << 2);
	// *ctx_ld |= ((int)1 << 1);
	// // *ctx_ld |= ((int)1 << 0);


	
//	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), sb_ctxld_trig | db_ctxld_trig); // this doesn't change it (taken from linux)
	//write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0xB001E); // no good leaves a bit of a gap
	

	//write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);
	// uint32_t* dtg_ctx_ld = (uint32_t*)(dcss_base + TC_CTX_LD_REG10);
	// *dtg_ctx_ld |= ((int)0 << 0);
	// *dtg_ctx_ld |= ((int)0 << 16);

	//write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), db_ctxld_trig);	// original 

	// int sb_ctxld_trig = ((0 * dis_lrc_y / 100) << TC_CTXLD_SB_Y_POS) &
	// 						TC_CTXLD_SB_Y_MASK;
	// int db_ctxld_trig = ((99 * dis_lrc_y / 100) << TC_CTXLD_DB_Y_POS) &
	// 						TC_CTXLD_DB_Y_MASK;

	// dcss_dtg_write(dtg, sb_ctxld_trig | db_ctxld_trig, DCSS_DTG_TC_CTXLD);

	// int ctxld_reg = 99 * (dis_lrc_y / 100);
	// uint32_t* ctxld_ptr = (dcss_base + TC_CTX_LD_REG10);
	// printf("Value before = %x\n", *ctxld_ptr);

	// write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), ctxld_reg);

	// printf("Value before = %x\n", *ctxld_ptr);

	// //ms_delay(100000);
	

	// In order to use these i need to configure the interrupts, otherwise they do nothing
	// I don't know how i would act upon it reaching these points

	// Is the idea that when it reaches this coordinate that it gets an interrupt and then i can action on it at that point?
	// if so thats what i tried above and it didn't really work as expected
	// Im thinking that the context loader register should behave in this way, so i set it at the point that i want it to trigger the context loader
	// In my version i just turn on the context loader whenever its ready - not anything to do with this status bit.
	// Should i be checking this as an interrupt?
	// The context loader doesn't give me an interrupt though... 

	// VBLANK
	//write_register((uint32_t*)(dcss_base + TC_LINE2_INT_REG14), 0x0000000);

	// // CTXLD
	//write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), ((90 * dis_lrc_y) / 100) << 16);
	//write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0);
}