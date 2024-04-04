#include <stdio.h>
#include <stdlib.h>

#include <microkit.h>


// UBOOT
#include <inttypes.h>
#include <address.h>
#include <externs.h>
#include <API_HDMITX.h>
#include <source_phy.h> 
#include <API_AVI.h>
#include <API_AFE_t28hpc_hdmitx.h>
#include <vic_table.h>
#include <API_General.h>

#include "dcss.h"
#include "dma.h"
#include "hdmi_data.h"
#include "dma_offsets.h"

#include "timer.h"

uintptr_t dcss_base;
uintptr_t dcss_blk_base;
uintptr_t gpc_base;
uintptr_t ccm_base;
uintptr_t dma_base;
uintptr_t dma_base_paddr;
uintptr_t timer_base;
uint32_t* current_frame_buffer_offset;

struct hdmi_data *hdmi_config = NULL;

int context = 0; // This keeps track of the current context. TODO: An alternative to global counter?

void init(void) {
	
	printf("Init Dcss\n");
	initialise_and_start_timer(timer_base);
	sel4_dma_init(dma_base_paddr, dma_base, dma_base + DMA_SIZE);
	
	/* Set the current buffer offset for the client to write to.
	 	This is here so that the client doesn't need to have a mechanism to decide which frame buffer to write to.
		Instead, the address of the the current frame buffer that is available to be written to to will be stored here.
		This will be set in the context loader and read from the client.
	*/
	uintptr_t* frame_buffer1_addr = getPhys((void*)dma_base);
	current_frame_buffer_offset = (uint32_t*)(dma_base + CURRENT_FRAME_BUFFER_ADDR_OFFSET);	
	
	// The client will set the frame buffer pointer to what ever is at this address. By default this is 0, which is at beginning of the DMA pool.
	*current_frame_buffer_offset = FRAME_BUFFER_ONE_OFFSET; 

	init_gpc();
	int* i = malloc(sizeof(int)); // TODO: This must be implemented so that the hdmi_data struct can be allocated new memory. (It will need to be freed then also)
	free(i);

}

void init_context_loader() {

	// Steps 1 and 2 of 15.4.2.2 Display state loading sequence are done here as the double buffered registers do not need to change what they contain.
	// So it should only be written once.
	
	uintptr_t* frame_buffer1_addr = getPhys((void*)dma_base);
	uintptr_t* frame_buffer2_addr = getPhys((void*)dma_base + FRAME_BUFFER_TWO_OFFSET);

	/*  
		The context loader has access to two double buffered registers depening on the current context.
		These registers are 64 bit and hold the address of the frame buffer in the first 32 bits and
		the DPR memory register where the frame buffer will be set in the second 32 bits. 
		See 15.4.2.3 System Memory Display state format
	*/
	uint32_t* ctx_ld_db1_addr = (uint32_t*)(dma_base + CTX_LD_DB_ONE_ADDR); 	
	*ctx_ld_db1_addr = (uintptr_t)frame_buffer1_addr;							
	ctx_ld_db1_addr++; 															
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 
	
	// This extra memory register has been added as the value is based on the current frame buffer
	ctx_ld_db1_addr++; 
	*ctx_ld_db1_addr = (uintptr_t)frame_buffer1_addr + (hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	ctx_ld_db1_addr++; 
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0; 


	uint32_t* ctx_ld_db2_addr = (uint32_t*)(dma_base + CTX_LD_DB_TWO_ADDR); 	
	*ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr;							
	ctx_ld_db2_addr++; 															
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 

	// This extra memory register has been added as the value is based on the current frame buffer
	ctx_ld_db2_addr++; 
	*ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr + (hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	ctx_ld_db2_addr++; 
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0; 
	
	run_context_loader();
}

void run_context_loader(){
	
	// Steps 3,4,5 and 12 of 15.4.2.2 Display state loading sequence

	printf("Running context loader in context: %d\n", context);
	start_timer();
	uint32_t* enable_status = (uint32_t*)(dcss_base + CTXLD_CTRL_STATUS);
	int context_ld_enabled = 0;
	
	// Give priority to the context loader TODO: Probably only needs to be done once per initialisation
	int arb_sel = (*enable_status >> 1) & (int)1;																
	
	// Set the context offset in memory for the current frame buffer to display
	int contex_offset = (context == 0) ? CTX_LD_DB_ONE_ADDR : CTX_LD_DB_TWO_ADDR;		


	// STEP 3 waiting until its idle (it will almost definitely just be idle already, but this is here just to follow the spec)
	context_ld_enabled = (*enable_status >> 0) & (int)1; 
	
	while (context_ld_enabled == 1) {																			
		context_ld_enabled = (*enable_status >> 0) & (int)1;
		//printf("test break\n");
		seL4_Yield();	
	}	

	// STEP 4 write the double buffered registers (values set previously in init_context_loader)
	// Set the base adress for the double buffered context
	write_register((uint32_t*)(dcss_base + DB_BASE_ADDR), (uintptr_t)getPhys((void*)dma_base + contex_offset));	
	write_register((uint32_t*)(dcss_base + DB_COUNT), 1);							
	
	// STEP 5 Set the context loader status to enable
	// Set the enable status bit to 1 to kickstart process.
	*enable_status |= ((int)1 << 0); 								// FIX: Switching the context is what causes the tear.														
	context_ld_enabled = (*enable_status >> 0) & (int)1; 
	
	// Poll contiously until context loader is not being used.
	while (context_ld_enabled == 1) {																			
		context_ld_enabled = (*enable_status >> 0) & (int)1;
		//printf("test break\n");
		seL4_Yield();	
	}

	// Set the dma offset for the current framebuffer to be used by the client
	*current_frame_buffer_offset = (context == 0) ? FRAME_BUFFER_TWO_OFFSET : FRAME_BUFFER_ONE_OFFSET;
	context = context == 1 ? 0 : 1; 																			
	printf("Switching context took %d ms\n", stop_timer());

	ms_delay(5000); // For debugging 

	// Notify the client to draw the frame buffer
	microkit_notify(52);
}

void
notified(microkit_channel ch) {
	
	switch (ch) {
		case 52:
			run_context_loader();
			break;
		case 55:
			reset_dcss();
			break;
		default:
			printf("Unexpected channel id: %d in dcss::notified() \n", ch);
	}
}

microkit_msginfo
protected(microkit_channel ch, microkit_msginfo msginfo) {
	
	printf("protected procedure called\n");
	switch (ch) {
		case 0:
		    hdmi_config = (struct hdmi_data *) microkit_msginfo_get_label(msginfo);
			init_dcss();
			return seL4_MessageInfo_new((uint64_t)hdmi_config,1,0,0); // what are the arguments?
			break;
		default:
			printf("Unexpected channel id: %d in dcss::protected() \n", ch);
	}
}

void init_dcss() {

	printf("init dcss called\n");
	if (hdmi_config != NULL) {
		printf("v data frequency = %d\n", hdmi_config->PIXEL_FREQ_KHZ);
	}
	else {
		printf("v data not yet set\n"); // TODO: This should then not go try init things, handle case properly 
	}

	init_ccm();
	reset_dcss();
	init_hdmi();
    write_dcss_memory_registers();

	if (hdmi_config->db_toggle) {
		init_context_loader();
	}
}

void init_ccm() {

	write_register((uint32_t*)(ccm_base + CCM_CCGR93_SET), 0x3);
	write_register((uint32_t*)(gpc_base + GPC_PGC_CPU_0_1_MAPPING), 0xffff); 
	write_register((uint32_t*)(gpc_base + GPC_PU_PGC_SW_PUP_REQ), 0x1 << 10);
}

void init_gpc() {

	write_register((uint32_t*)(ccm_base + CCM_TARGET_ROOT20), 0x12000000); 
	write_register((uint32_t*)(ccm_base + CCM_TARGET_ROOT22), 0x11010000);
}

void reset_dcss(){
	
	write_register((uint32_t*)(dcss_blk_base), 0xffffffff);
	write_register((uint32_t*)(dcss_blk_base + CONTROL0), 0x1);
	write_register((uint32_t*)(dcss_base +  TC_CONTROL_STATUS), 0); 
	write_register((uint32_t*)(dcss_base +  SCALE_CTRL), 0);
	write_register((uint32_t*)(dcss_base +  SCALE_OFIFO_CTRL), 0);
	write_register((uint32_t*)(dcss_base +  SCALE_SRC_DATA_CTRL), 0);
	write_register((uint32_t*)(dcss_base +  DPR_1_SYSTEM_CTRL0), 0);
	write_register((uint32_t*)(dcss_base +  SS_SYS_CTRL), 0);
}

void init_hdmi() {
	
	uint8_t bits_per_pixel = 8; // 8 = 24 + alpha, 10= 30 +alpha
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = PXL_RGB;

	if (init_api() == CDN_OK) {
		uint32_t phy_frequency = phy_cfg_t28hpc(4, hdmi_config->PIXEL_FREQ_KHZ, bits_per_pixel, pixel_encoding_format, 1);
		hdmi_tx_t28hpc_power_config_seq(4);
		call_api(phy_frequency, pixel_encoding_format, bits_per_pixel); // TODO: handle the return
	}
	else {
		printf("Failed to initialise API ensure the hdmi firmware is enabled in your boot loader.\n");
	}
}

CDN_API_STATUS init_api() {

	CDN_API_STATUS api_status = CDN_OK;
	
	cdn_api_init();
	
	api_status = cdn_api_checkalive();
	print_api_status_msg(api_status, "cdn_api_checkalive()");
	
	uint8_t test_message[] = "test message";
	uint8_t test_response[sizeof(test_message) + 1];
	
	api_status = cdn_api_general_test_echo_ext_blocking(test_message,
														test_response,
														sizeof(test_message),
														CDN_BUS_TYPE_APB);
	print_api_status_msg(api_status, "cdn_api_general_test_echo_ext_blocking()");						
	
	return api_status;
}

void call_api(uint32_t phy_frequency, VIC_PXL_ENCODING_FORMAT pixel_encoding_format, uint8_t bits_per_pixel) {
	
	CDN_API_STATUS api_status = CDN_OK;   
	BT_TYPE bt_type = 0;
	HDMI_TX_MAIL_HANDLER_PROTOCOL_TYPE protocol_type = 1;

	api_status = cdn_api_general_write_register_blocking
		(ADDR_SOURCD_PHY + (LANES_CONFIG << 2),
		 F_SOURCE_PHY_LANE0_SWAP(0) | F_SOURCE_PHY_LANE1_SWAP(1) |
		 F_SOURCE_PHY_LANE2_SWAP(2) | F_SOURCE_PHY_LANE3_SWAP(3) |
		 F_SOURCE_PHY_COMB_BYPASS(0) | F_SOURCE_PHY_20_10(1));
	print_api_status_msg(api_status, "cdn_api_general_write_register_blocking");

	api_status = CDN_API_HDMITX_Init_blocking();
	print_api_status_msg(api_status, "CDN_API_HDMITX_Init_blocking");
	
	api_status = CDN_API_HDMITX_Set_Mode_blocking(protocol_type, phy_frequency);
	print_api_status_msg(api_status, "CDN_API_HDMITX_Set_Mode_blocking");

	api_status = cdn_api_set_avi(hdmi_config, pixel_encoding_format, bt_type);
	print_api_status_msg(api_status, "cdn_api_set_avi");

	api_status = CDN_API_HDMITX_SetVic_blocking(hdmi_config, bits_per_pixel, pixel_encoding_format);
	print_api_status_msg(api_status, "CDN_API_HDMITX_SetVic_blocking");

	// TODO: Potentially need timer here
	// TODO: This only prints if one of these calls fail - it should potentially return something to say at least one of them has returned with a non 0 status.
}

void print_api_status_msg(CDN_API_STATUS status, char* function_name) {
	
	if (status != CDN_OK){
		printf("%s returned non 0 status %d\n", function_name, status);
	}
	else {
		printf("%s returned successfully\n", function_name);
	}
}

void write_dcss_memory_registers() {
	
	write_dtrc_memory_registers();
	write_dpr_memory_registers();
	write_scaler_memory_registers();
	write_sub_sampler_memory_registers();
	write_dtg_memory_registers();
}

void write_dtrc_memory_registers() {
    
    write_register((uint32_t*)(dcss_base + DTCTRL_CHAN2), 0x00000002);
    write_register((uint32_t*)(dcss_base + DTCTRL_CHAN3), 0x00000002);
}

void write_dpr_memory_registers() {
	
	uintptr_t* dma_addr =  getPhys((void*) (dma_base));

    write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr); // The address of the frame buffer
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	write_register((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

	write_register((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004);
	write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000005); // can this bit be set?
}

void write_sub_sampler_memory_registers() {

	write_register((uint32_t*)(dcss_base + SS_COEFF), 0x21612161);
	write_register((uint32_t*)(dcss_base + SS_CLIP_CB), 0x03ff0000);
	write_register((uint32_t*)(dcss_base + SS_CLIP_CR), 0x03ff0000);


	// TODO: Tidy these up
	write_register((uint32_t*)(dcss_base + SS_DISPLAY),
		    (((hdmi_config->TYPE_EOF + hdmi_config->SOF +  hdmi_config->VSYNC +
			hdmi_config->V_ACTIVE -1) << 16) |
		       (hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->HSYNC+
			hdmi_config->H_ACTIVE - 1)));
	write_register((uint32_t*)(dcss_base + SS_HSYNC),
		    (((hdmi_config->HSYNC- 1) << 16) | (hdmi_config->HSYNC_POL != 0) << 31 | (hdmi_config->FRONT_PORCH +
			hdmi_config->BACK_PORCH + hdmi_config->HSYNC+ hdmi_config->H_ACTIVE -1)));
	write_register((uint32_t*)(dcss_base + SS_VSYNC),
		    (((hdmi_config->TYPE_EOF +  hdmi_config->VSYNC - 1) << 16) | (hdmi_config->VSYNC_POL != 0) << 31 | (hdmi_config->TYPE_EOF - 1)));
	write_register((uint32_t*)(dcss_base + SS_DE_ULC),
		    ((1 << 31) | (( hdmi_config->VSYNC +hdmi_config->TYPE_EOF + hdmi_config->SOF) << 16) |
		    (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH - 1)));
	write_register((uint32_t*)(dcss_base + SS_DE_LRC),
		    ((( hdmi_config->VSYNC + hdmi_config->TYPE_EOF + hdmi_config->SOF + hdmi_config->V_ACTIVE -1) << 16) |
		    (hdmi_config->HSYNC+ hdmi_config->BACK_PORCH + hdmi_config->H_ACTIVE - 1)));

	write_register((uint32_t*)(dcss_base + SS_MODE), 0x0000000);
	write_register((uint32_t*)(dcss_base + SS_SYS_CTRL), 0x00000001);
}

void write_dtg_memory_registers() {
	
	int dis_lrc_y = hdmi_config->VSYNC + hdmi_config->FRONT_PORCH + hdmi_config->BACK_PORCH + hdmi_config->V_ACTIVE - 1; // rename 
	
	if (hdmi_config->alpha_toggle == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005484); // TODO: add defines
	}
	else {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);
	}


	// TODO: Tidy these up
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
	

	// This needs re writing - its current settings mean it can switch properly between contexts, but may need to be written with hdmi_config values.
	write_register((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);
	

	// These two may actually be things set in the context loader registers. (I'm not sure why though, i would have thought its only needed ot be set once.)

	// VBLANK
	write_register((uint32_t*)(dcss_base + TC_LINE2_INT_REG14), 0x0000000);

	// CTXLD
	write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), ((90 * dis_lrc_y) / 100) << 16);


	if (hdmi_config->alpha_toggle == ALPHA_ON) {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584); // TODO: add defines
	}
	else {
		write_register((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); 
	}
}

void write_scaler_memory_registers() {

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
}