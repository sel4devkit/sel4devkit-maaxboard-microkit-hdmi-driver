#include <microkit.h>
#include <stdio.h>
#include <stdlib.h>

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

uintptr_t dcss_base;
uintptr_t dcss_blk_base;
uintptr_t gpc_base;
uintptr_t ccm_base;
uintptr_t rc_base;
uintptr_t frame_buffer;
uintptr_t frame_buffer_paddr;

struct hdmi_data *v_data	= NULL;

#define FRAME_BUFFER_SIZE 1280 * 720 * 4 // TODO: re define  



void init(void) {
	
	printf("Init Dcss\n");
	sel4_dma_init(frame_buffer_paddr, frame_buffer, frame_buffer + FRAME_BUFFER_SIZE);
	init_gpc();
}

void
notified(microkit_channel ch) {
	
	switch (ch) {
        case 46:
			init_dcss();
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
		printf("v data not yet set\n");
	}

	init_ccm();
	reset_dcss();
	init_hdmi();
    write_dcss_memory_registers();
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
	
	uintptr_t* dma_addr =  getPhys((void*)frame_buffer);

    write_32bit_to_mem_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), v_data->H_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), v_data->V_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr + v_data->H_ACTIVE * v_data->V_ACTIVE);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((v_data->H_ACTIVE * 4) << 16));
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), 0x000e4203); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

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
	
	// disable local alpha
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);
	
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

	// disable local alpha
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);
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