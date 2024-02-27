#include <microkit.h>
#include <stdio.h>
#include <stdlib.h>

#include <dcss.h>
#include <dma.h>

// UBOOT
#include <inttypes.h>
#include <address.h>
#include <externs.h>
#include <API_HDMITX.h>
#include <source_phy.h> 
#include "API_AVI.h"
#include <API_AFE_t28hpc_hdmitx.h>
#include <vic_table.h>
#include "API_General.h"

#include <hdmi_data.h>

uintptr_t dcss_base;
uintptr_t dcss_blk_base;
uintptr_t gpc_base;
uintptr_t ccm_base;
uintptr_t rc_base;
uintptr_t frame_buffer;
uintptr_t frame_buffer_paddr;





int glob_mode = 0;

struct hdmi_information hdmi_info; // does this persist through each notify call? Or is it created again each time?

struct vic_data *v_data	= NULL;
struct vic_data *v_data_other;

#define FRAME_BUFFER_SIZE 1280 * 720 * 4 // TODO: re define  


//void v_intr(void* v);

void init(void) {

	printf("Init DCSS\n");
	sel4_dma_init(frame_buffer_paddr, frame_buffer, frame_buffer + FRAME_BUFFER_SIZE);

	// could initialise a default configuration here - maybe leave it up to the client though... 
	//init_dcss(2);

	//malloc(2);

}

void
notified(microkit_channel ch) {

	printf("notified\n");
	switch (ch) {
        case 46:
			init_dcss(glob_mode);
			glob_mode++;
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
		    v_data = (struct vic_data *) microkit_msginfo_get_label(msginfo);
			return seL4_MessageInfo_new((uint64_t)v_data,1 ,0,0); // why?
			break;
		default:
			printf("Unexpected channel id: %d in dcss::protected() \n", ch);
	}
}



void init_dcss(int vic_mode) {

	printf("init dcss called\n");
	if (v_data != NULL) {
		printf("v data A = %d\n", v_data->a);
		//v_intr(v_data);
	}
	else {
		printf("v data not yet set\n");
	}

	write_32bit_to_mem((uint32_t*)(dcss_base +  0x20000), 0);
	/* scaler */
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x1c000), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x1c400), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x1c800), 0);
	/* dpr */
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x18000), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x19000), 0);
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x1a000), 0);
	/* sub-sampler*/
	write_32bit_to_mem((uint32_t*)(dcss_base +  0x1b000), 0);


	init_ccm(); // ccm and gpc may not need to be set here (doesn't really matter, but worth noting)
	init_gpc();
	hdmi_set_timings(&hdmi_info, vic_mode); // instead of using the vic table, we can instead provide the user with the option of providing these themselves (but also provide an option of using the vic table to get these values)
	reset_dcss();
	init_hdmi(vic_mode);
    write_dcss_memory_registers();
}

void hdmi_set_timings(struct hdmi_information* hdmi_info, int vic_mode) {
	
	hdmi_info->timings.hfront_porch.typ = vic_table[vic_mode][FRONT_PORCH];
	hdmi_info->timings.hback_porch.typ = vic_table[vic_mode][BACK_PORCH];
	hdmi_info->timings.hsync_len.typ = vic_table[vic_mode][HSYNC];
	hdmi_info->timings.vfront_porch.typ = vic_table[vic_mode][TYPE_EOF];
	hdmi_info->timings.vback_porch.typ = vic_table[vic_mode][SOF];
	hdmi_info->timings.vsync_len.typ = vic_table[vic_mode][VSYNC];
	hdmi_info->timings.hactive.typ = vic_table[vic_mode][H_ACTIVE];
	hdmi_info->timings.vactive.typ = vic_table[vic_mode][V_ACTIVE];
	hdmi_info->horizontal_pulse_polarity = vic_table[vic_mode][HSYNC_POL] != 0;
	hdmi_info->vertical_pulse_polarity = vic_table[vic_mode][VSYNC_POL] != 0;
	hdmi_info->timings.pixelclock.typ = vic_table[vic_mode][PIXEL_FREQ_KHZ] * 1000;
}

void init_ccm() {

	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_CCGR93_SET), 0x3);
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PGC_CPU_0_1_MAPPING), 0xffff); 
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PU_PGC_SW_PUP_REQ), 0x1 << 10);
}

void init_gpc() {
	// GPC
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT20), 0x12000000); 
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT22), 0x11010000);
}

void reset_dcss(){

	// DCSS BLK 
	write_32bit_to_mem((uint32_t*)(dcss_blk_base), 0xffffffff);
	write_32bit_to_mem((uint32_t*)(dcss_blk_base + CONTROL0), 0x1);
}

void init_hdmi(int vic_mode) {
	
	uint8_t bits_per_pixel = 8; // 8 actualyl goes down as 32 (this has no affect)
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = PXL_RGB;

	init_api(); // TODO: handle the return

	uint32_t phy_frequency = phy_cfg_t28hpc(4, vic_mode, bits_per_pixel, pixel_encoding_format, 1);
	hdmi_tx_t28hpc_power_config_seq(4);

	call_api(phy_frequency, vic_mode, pixel_encoding_format, bits_per_pixel); // TODO: handle the return
}

CDN_API_STATUS init_api() {

	CDN_API_STATUS api_status = CDN_OK;
	
	cdn_api_init();
	api_status = cdn_api_checkalive();
	uint8_t test_message[] = "test message";
	uint8_t test_response[sizeof(test_message) + 1];
	api_status = cdn_api_general_test_echo_ext_blocking(test_message,
														test_response,
														sizeof(test_message),
														CDN_BUS_TYPE_APB);
	return api_status;
}

CDN_API_STATUS call_api(uint32_t phy_frequency, VIC_MODES vic_mode, VIC_PXL_ENCODING_FORMAT pixel_encoding_format, uint8_t bits_per_pixel) {
	
	CDN_API_STATUS api_status = CDN_OK;   
	BT_TYPE bt_type = 0;
	HDMI_TX_MAIL_HANDLER_PROTOCOL_TYPE protocol_type = 1;

	api_status = cdn_api_general_write_register_blocking
		(ADDR_SOURCD_PHY + (LANES_CONFIG << 2),
		 F_SOURCE_PHY_LANE0_SWAP(0) | F_SOURCE_PHY_LANE1_SWAP(1) |
		 F_SOURCE_PHY_LANE2_SWAP(2) | F_SOURCE_PHY_LANE3_SWAP(3) |
		 F_SOURCE_PHY_COMB_BYPASS(0) | F_SOURCE_PHY_20_10(1));

	api_status = CDN_API_HDMITX_Init_blocking();
	api_status = CDN_API_HDMITX_Init_blocking();
	api_status = CDN_API_HDMITX_Set_Mode_blocking(protocol_type, phy_frequency);
	api_status = cdn_api_set_avi(vic_mode, pixel_encoding_format, bt_type);
	api_status = CDN_API_HDMITX_SetVic_blocking(vic_mode, bits_per_pixel, pixel_encoding_format);

	// TODO: Potentially need timer here

	return api_status;
}


void write_dcss_memory_registers() {
	
	write_dtrc_memory_registers();
	write_dpr_memory_registers(&hdmi_info);
	write_scaler_memory_registers(&hdmi_info);
	write_sub_sampler_memory_registers(&hdmi_info);
	write_dtg_memory_registers(&hdmi_info);
}

void write_dtrc_memory_registers() {
    
	// DTRC-CHAN2/3
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN2), 0x00000002);
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN3), 0x00000002);
}

void write_dpr_memory_registers(struct hdmi_information* hdmi_info) {
	
	uintptr_t* dma_addr =  getPhys((void*)frame_buffer);
	// printf("dma phys addr =  %" PRIxPTR "\n", (uintptr_t)dma_addr);
	// printf("dma base addr =  %" PRIxPTR "\n", frame_buffer);
	
	//  Chan1_DPR
    write_32bit_to_mem_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), hdmi_info->timings.hactive.typ);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), hdmi_info->timings.vactive.typ);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)dma_addr + hdmi_info->timings.hactive.typ * hdmi_info->timings.vactive.typ);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((hdmi_info->timings.hactive.typ * 4) << 16));
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), 0x000e4203);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000005); // can this bit be set?
}

void write_sub_sampler_memory_registers(struct hdmi_information* hdmi_info) {

	/* SUBSAM */
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_COEFF), 0x21612161);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_CLIP_CB), 0x03ff0000);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_CLIP_CR), 0x03ff0000);

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DISPLAY),
		    (((hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ + hdmi_info->timings.vsync_len.typ +
			hdmi_info->timings.vactive.typ -1) << 16) |
		       (hdmi_info->timings.hfront_porch.typ + hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hsync_len.typ +
			hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_HSYNC),
		    (((hdmi_info->timings.hsync_len.typ - 1) << 16) | hdmi_info->horizontal_pulse_polarity << 31 | (hdmi_info->timings.hfront_porch.typ +
			hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hactive.typ -1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_VSYNC),
		    (((hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vsync_len.typ - 1) << 16) | hdmi_info->vertical_pulse_polarity << 31 | (hdmi_info->timings.vfront_porch.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DE_ULC),
		    ((1 << 31) | ((hdmi_info->timings.vsync_len.typ +hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ) << 16) |
		    (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_DE_LRC),
		    (((hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ + hdmi_info->timings.vactive.typ -1) << 16) |
		    (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hactive.typ - 1)));

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_MODE), 0x0000000);
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_SYS_CTRL), 0x00000001);
}

void write_dtg_memory_registers(struct hdmi_information* hdmi_info) {
	
	// disable local alpha
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005084);
	
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DTG_REG1),
		    (((hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ + hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vactive.typ -
		       1) << 16) | (hdmi_info->timings.hfront_porch.typ + hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hsync_len.typ +
			hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DISPLAY_REG2),
		    (((hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ -
		       1) << 16) | (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_DISPLAY_REG3),
		    (((hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ + hdmi_info->timings.vactive.typ -
		       1) << 16) | (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CH1_REG4),
		    (((hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ -
		       1) << 16) | (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CH1_REG5),
		    (((hdmi_info->timings.vsync_len.typ + hdmi_info->timings.vfront_porch.typ + hdmi_info->timings.vback_porch.typ + hdmi_info->timings.vactive.typ -
		       1) << 16) | (hdmi_info->timings.hsync_len.typ + hdmi_info->timings.hback_porch.typ + hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CTX_LD_REG10), 0x000b000a);

	// disable local alpha
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184);
}


void write_scaler_memory_registers(struct hdmi_information* hdmi_info) {

	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c008), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c00c), 0x00000000); // 8bit colour depth 
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c010), 0x00000002);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c014), 0x00000002);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c018),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c01c),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c020),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c024),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c028), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c02c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c030), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c034), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c038), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c03c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c040), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c044), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c048), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c04c), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c050), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c054), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c058), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c05c), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c060), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c064), 0x00002000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c080), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0c0), 0x00040000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c100), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c084), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0c4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c104), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c088), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0c8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c108), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c08c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0cc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c10c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c090), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0d0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c110), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c094), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0d4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c114), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c098), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0d8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c118), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c09c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0dc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c11c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0a0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0e0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c120), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0a4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0e4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c124), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0a8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0e8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c128), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0ac), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0ec), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c12c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0b0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0f0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c130), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0b4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0f4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c134), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0b8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0f8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c138), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0bc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c0fc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c13c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c140), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c180), 0x00040000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1c0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c144), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c184), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1c4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c148), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c188), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1c8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c14c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c18c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1cc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c150), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c190), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1d0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c154), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c194), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1d4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c158), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c198), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1d8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c15c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c19c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1dc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c160), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1a0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1e0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c164), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1a4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1e4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c168), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1a8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1e8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c16c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1ac), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1ec), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c170), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1b0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1f0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c174), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1b4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1f4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c178), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1b8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1f8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c17c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1bc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c1fc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c300), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c340), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c380), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c304), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c344), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c384), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c308), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c348), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c388), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c30c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c34c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c38c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c310), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c350), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c390), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c314), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c354), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c394), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c318), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c358), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c398), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c31c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c35c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c39c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c320), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c360), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3a0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c324), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c364), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3a4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c328), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c368), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3a8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c32c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c36c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3ac), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c330), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c370), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3b0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c334), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c374), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3b4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c338), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c378), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3b8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c33c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c37c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c3bc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c200), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c240), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c280), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c204), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c244), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c284), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c208), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c248), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c288), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c20c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c24c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c28c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c210), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c250), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c290), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c214), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c254), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c294), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c218), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c258), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c298), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c21c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c25c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c29c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c220), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c260), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2a0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c224), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c264), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2a4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c228), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c268), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2a8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c22c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c26c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2ac), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c230), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c270), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2b0), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c234), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c274), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2b4), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c238), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c278), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2b8), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c23c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c27c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2bc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c2bc), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c000), 0x00000011);
}