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
uintptr_t rc_base; // where was this needed?
uintptr_t dma_base;
uintptr_t dma_base_paddr;
uintptr_t timer_base;

struct hdmi_data *v_data = NULL;

int context = 0; // An alternative to global counter?

void init(void) {
	
	printf("Init Dcss\n");
	initialise_and_start_timer(timer_base);
	sel4_dma_init(dma_base_paddr, dma_base, dma_base + CTX_LD_DMA_SIZE); // This is too big and needs to be thought of more carefully.
	init_gpc();
	int* i = malloc(sizeof(int)); // hack not needed
	free(i);
}

void init_context_loader() {

	uintptr_t* frame_buffer1_addr = getPhys((void*)dma_base);
	uintptr_t* frame_buffer2_addr = getPhys((void*)dma_base + FRAME_BUFFER_TWO_OFFSET);

	uint32_t* ctx_ld_db1_addr = (uint32_t*)(dma_base + CTX_LD_DB_ONE_ADDR); 	// This needs to be an offset from the dma base (frame buffer size *2)
	*ctx_ld_db1_addr = (uint32_t)frame_buffer1_addr;							// Set the address of the first frame buffer
	ctx_ld_db1_addr++; 															// Increase pointer by 32 bits
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 				// The memory register that we are changing (the DPR Address)

	uint32_t* ctx_ld_db2_addr = (uint32_t*)(dma_base + CTX_LD_DB_TWO_ADDR); 	// This needs to be an offset from the dma base (frame buffer size *2) + something
	*ctx_ld_db2_addr = (uint32_t)frame_buffer2_addr;							// Set the address of the second frame buffer
	ctx_ld_db2_addr++; 															// Increase pointer by 32 bits
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 				// The memory register that we are changing (the DPR Address)
	
	run_context_loader();
}

void run_context_loader(){
	printf("Running context loader in context: %d\n", context);
	uint32_t* enable_status = (uint32_t*)(dcss_base + CTXLD_CTRL_STATUS);
	int context_ld_enabled = 0;

	int arb_sel = (*enable_status >> 1) & (int)1;

	int contex_offset = (context == 0) ? CTX_LD_DB_ONE_ADDR : CTX_LD_DB_TWO_ADDR;							// Set the context offset in memory
	write_32bit_to_mem((uint32_t*)(dcss_base + DB_BASE_ADDR), getPhys((void*)dma_base + contex_offset));	// set the base adress for the double buffered context
	write_32bit_to_mem((uint32_t*)(dcss_base + DB_COUNT), 1);												// Set how many registers are being processed
	
	*enable_status |= ((int)1 << 0); 																		// set the enable status bit to 1 to kickstart process.
	context_ld_enabled = (*enable_status >> 0) & (int)1; 
	while (context_ld_enabled == 1) {																		// poll contiously until context loader is not being used.
		printf("context  = %d\n", context_ld_enabled); 														// If this print statement isn't here it doesn't do anything... FIX THIS
		context_ld_enabled = (*enable_status >> 0) & (int)1;
	}

	context = context == 1 ? 0 : 1; 																		// Switch context for next time
	microkit_notify(52);
}

void
notified(microkit_channel ch) {
	
	switch (ch) {
        case 46:
			init_dcss();
			break;
		case 52:
			run_context_loader();
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
		    v_data = (struct hdmi_data *) microkit_msginfo_get_label(msginfo);
			// init_dcss(); // This is a possibility instead of going through the notified function. It may not reach the return statement though
			// Instead of the case 46 for notified, it could call this and pass in the v_data struct. That way the v_data struct doesn't need to be malloced.
			return seL4_MessageInfo_new((uint64_t)v_data,1 ,0,0); // why?
			break;
		default:
			printf("Unexpected channel id: %d in dcss::protected() \n", ch);
	}
}

void init_dcss() {

	printf("init dcss called\n");
	if (v_data != NULL) {
		printf("v data frequency = %d\n", v_data->PIXEL_FREQ_KHZ);
	}
	else {
		printf("v data not yet set\n"); // This should then not go try init things, handle case properly 
	}

	init_ccm();
	reset_dcss();
	init_hdmi();
    write_dcss_memory_registers();

	if (v_data->db_toggle) {
		init_context_loader();
	}
}

void init_ccm() {

	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_CCGR93_SET), 0x3);
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PGC_CPU_0_1_MAPPING), 0xffff); 
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PU_PGC_SW_PUP_REQ), 0x1 << 10);
}

void init_gpc() {

	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT20), 0x12000000); 
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT22), 0x11010000);
}

void reset_dcss(){
	
	write_32bit_to_mem((uint32_t*)(dcss_blk_base), 0xffffffff);
	write_32bit_to_mem((uint32_t*)(dcss_blk_base + CONTROL0), 0x1);
	write_32bit_to_mem((uint32_t*)(dcss_base +  TC_CONTROL_STATUS), 0); 
	write_32bit_to_mem((uint32_t*)(dcss_base +  SCALE_CTRL), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  SCALE_OFIFO_CTRL), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  SCALE_SRC_DATA_CTRL), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  DPR_1_SYSTEM_CTRL0), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  SS_SYS_CTRL), 0);
}

void init_hdmi() {
	
	uint8_t bits_per_pixel = 8; // 8 = 24 + alpha, 10= 30 +alpha
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = PXL_RGB;

	if (init_api() == CDN_OK) {
		uint32_t phy_frequency = phy_cfg_t28hpc(4, v_data->PIXEL_FREQ_KHZ, bits_per_pixel, pixel_encoding_format, 1);
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

	api_status = cdn_api_set_avi(v_data, pixel_encoding_format, bt_type);
	print_api_status_msg(api_status, "cdn_api_set_avi");

	api_status = CDN_API_HDMITX_SetVic_blocking(v_data, bits_per_pixel, pixel_encoding_format);
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
    
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN2), 0x00000002);
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN3), 0x00000002);
}

void write_dpr_memory_registers() {
	
	uintptr_t* dma_addr =  getPhys((void*) (dma_base));

    write_32bit_to_mem_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), v_data->H_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), v_data->V_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr + v_data->H_ACTIVE * v_data->V_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((v_data->H_ACTIVE * 4) << 16));
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), v_data->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000005); // can this bit be set?
}

void write_sub_sampler_memory_registers() {

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_COEFF), 0x21612161);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_CLIP_CB), 0x03ff0000);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_CLIP_CR), 0x03ff0000);

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DISPLAY),
		    (((v_data->TYPE_EOF + v_data->SOF +  v_data->VSYNC +
			v_data->V_ACTIVE -1) << 16) |
		       (v_data->FRONT_PORCH + v_data->BACK_PORCH + v_data->HSYNC+
			v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_HSYNC),
		    (((v_data->HSYNC- 1) << 16) | (v_data->HSYNC_POL != 0) << 31 | (v_data->FRONT_PORCH +
			v_data->BACK_PORCH + v_data->HSYNC+ v_data->H_ACTIVE -1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_VSYNC),
		    (((v_data->TYPE_EOF +  v_data->VSYNC - 1) << 16) | (v_data->VSYNC_POL != 0) << 31 | (v_data->TYPE_EOF - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DE_ULC),
		    ((1 << 31) | (( v_data->VSYNC +v_data->TYPE_EOF + v_data->SOF) << 16) |
		    (v_data->HSYNC+ v_data->BACK_PORCH - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DE_LRC),
		    ((( v_data->VSYNC + v_data->TYPE_EOF + v_data->SOF + v_data->V_ACTIVE -1) << 16) |
		    (v_data->HSYNC+ v_data->BACK_PORCH + v_data->H_ACTIVE - 1)));

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_MODE), 0x0000000);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_SYS_CTRL), 0x00000001);
}

void write_dtg_memory_registers() {
	
	if (v_data->alpha_toggle == ALPHA_ON) {
		write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005484);
	}
	else {
		write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);
	}
	
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DTG_REG1),
		    (((v_data->TYPE_EOF + v_data->SOF +  v_data->VSYNC + v_data->V_ACTIVE -
		       1) << 16) | (v_data->FRONT_PORCH + v_data->BACK_PORCH + v_data->HSYNC+
			v_data->H_ACTIVE - 1)));	
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
		    ((( v_data->VSYNC + v_data->TYPE_EOF + v_data->SOF -
		       1) << 16) | (v_data->HSYNC+ v_data->BACK_PORCH - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
		    ((( v_data->VSYNC + v_data->TYPE_EOF + v_data->SOF + v_data->V_ACTIVE -
		       1) << 16) | (v_data->HSYNC+ v_data->BACK_PORCH + v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CH1_REG4),
		    ((( v_data->VSYNC + v_data->TYPE_EOF + v_data->SOF -
		       1) << 16) | (v_data->HSYNC+ v_data->BACK_PORCH - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CH1_REG5),
		    ((( v_data->VSYNC + v_data->TYPE_EOF + v_data->SOF + v_data->V_ACTIVE -
		       1) << 16) | (v_data->HSYNC+ v_data->BACK_PORCH + v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	if (v_data->alpha_toggle == ALPHA_ON) {
		write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xFF005584);
	}
	else {
		write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); 
	}
}

void write_scaler_memory_registers() {

	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_SRC_DATA_CTRL), 0x00000000); // This must stay!
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_SRC_FORMAT), 0x00000002); // Sets to RGB
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_DST_FORMAT), 0x00000002); // Sets to RGB
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_SRC_LUMA_RES),
		    ((v_data->V_ACTIVE - 1) << 16 | (v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_SRC_CHROMA_RES),
		    ((v_data->V_ACTIVE - 1) << 16 | (v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c020),
		    ((v_data->V_ACTIVE - 1) << 16 | (v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_DST_CHROMA_RES),
		    ((v_data->V_ACTIVE - 1) << 16 | (v_data->H_ACTIVE - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_V_LUMA_INC), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_H_LUMA_INC), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_V_CHROMA_INC), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + SCALE_H_CHROMA_INC), 0x00002000);


	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0c0), 0x00040000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c140), 0x00000000); // This must stay!
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c180), 0x00040000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1c0), 0x00000000); // This must stay!
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c000), 0x00000011);
}