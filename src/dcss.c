#include <microkit.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <dcss.h>
#include <dma.h>

// UBOOT
#include <inttypes.h>
#include <vic_table.h>
#include <externs.h>
#include "API_General.h"
#include <API_AFE_t28hpc_hdmitx.h>


uintptr_t dcss_base;
uintptr_t dcss_blk_base;
uintptr_t gpc_base;
uintptr_t ccm_base;
uintptr_t rc_base;
uintptr_t frame_buffer;
uintptr_t frame_buffer_paddr;

struct hdmi_information hdmi_info; // does this persist through each notify call? Or is it created again each time?

#define FRAME_BUFFER_SIZE 1280 * 720 * 4 // re define 
#define VIC_MODE 2 // Understand different vic modes

void init(void) {
    
	printf("Init DCSS UPDATE\n");
	sel4_dma_init(frame_buffer_paddr, frame_buffer, frame_buffer + FRAME_BUFFER_SIZE); // init frame buffer
    hdmi_set_timings(&hdmi_info);
	init_dcss();
	init_hdmi();
    write_dcss_memory_registers();
}

void
notified(microkit_channel ch) {
}

// imx8m_hdmi_set_vic_mode() - uboot-imx/drivers/video/nxp/imx/hdmi/imx8m_hdmi.c
void hdmi_set_timings(struct hdmi_information* hdmi_info) {

    uint32_t pixel_clock_frequency;
	hdmi_info->timings.hfront_porch.typ = vic_table[VIC_MODE][FRONT_PORCH];
	hdmi_info->timings.hback_porch.typ = vic_table[VIC_MODE][BACK_PORCH];
	hdmi_info->timings.hsync_len.typ = vic_table[VIC_MODE][HSYNC];
	hdmi_info->timings.vfront_porch.typ = vic_table[VIC_MODE][TYPE_EOF];
	hdmi_info->timings.vback_porch.typ = vic_table[VIC_MODE][SOF];
	hdmi_info->timings.vsync_len.typ = vic_table[VIC_MODE][VSYNC];
	hdmi_info->timings.hactive.typ = vic_table[VIC_MODE][H_ACTIVE];
	hdmi_info->timings.vactive.typ = vic_table[VIC_MODE][V_ACTIVE];
	hdmi_info->horizontal_pulse_polarity = vic_table[VIC_MODE][HSYNC_POL] != 0;
	hdmi_info->vertical_pulse_polarity = vic_table[VIC_MODE][VSYNC_POL] != 0;
	pixel_clock_frequency    = vic_table[VIC_MODE][PIXEL_FREQ_KHZ];
	hdmi_info->timings.pixelclock.typ = pixel_clock_frequency * 1000;
}

void init_dcss(){

	// CCM
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_CCGR93_SET), 0x3);
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PGC_CPU_0_1_MAPPING), 0xffff); 
	write_32bit_to_mem((uint32_t*)(gpc_base + GPC_PU_PGC_SW_PUP_REQ), 0x1 << 10);

	// GPC
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT20), 0x12000000); 
	write_32bit_to_mem((uint32_t*)(ccm_base + CCM_TARGET_ROOT22), 0x11010000);

	// DCSS BLK 
	write_32bit_to_mem((uint32_t*)(dcss_blk_base), 0xffffffff);
	write_32bit_to_mem((uint32_t*)(dcss_blk_base + CONTROL0), 0x1);
}


void init_hdmi() {
	
	uint8_t bits_per_pixel = 8;
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = 1;
	uint32_t character_freq_khz = phy_cfg_t28hpc(4, VIC_MODE, bits_per_pixel, pixel_encoding_format, 1);
	hdmi_tx_t28hpc_power_config_seq(4);
}


void write_dcss_memory_registers() {
	
	write_dtrc_memory_registers();
	write_dpr_memory_registers(&hdmi_info);
	write_sub_sampler_memory_registers(&hdmi_info);
	write_dtg_memory_registers(&hdmi_info);
	//write_scaler_memory_registers(&hdmi_info);
}

void write_dtrc_memory_registers() {
    
	// DTRC-CHAN2/3
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN2), 0x00000002);
    write_32bit_to_mem((uint32_t*)(dcss_base + DTCTRL_CHAN3), 0x00000002);
}

void write_dpr_memory_registers(struct hdmi_information* hdmi_info) {
	
	uintptr_t* dma_addr =  getPhys((void*)frame_buffer);
	printf("dma phys addr =  %" PRIxPTR "\n", (uintptr_t)dma_addr);
	printf("dma base addr =  %" PRIxPTR "\n", frame_buffer);
	
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
	// write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), 0x000e4203);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004);
	// write_32bit_to_mem((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000005); // can this bit be set?
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

	/* subsample mode 0 bypass 444, 1 422, 2 420 */
	write_32bit_to_mem((uint32_t*)(dcss_base + SS_MODE), 0x0000000);

	write_32bit_to_mem((uint32_t*)(dcss_base + SS_SYS_CTRL), 0x00000001);
}

void write_dtg_memory_registers(struct hdmi_information* hdmi_info) {
	
	/* DTG */
	/*write_32bit_to_mem((uint32_t*)(dcss_base + 0x20000, 0xff000484); */
	/* disable local alpha */
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

	/* disable local alpha */
	write_32bit_to_mem((uint32_t*)(dcss_base + TC_CONTROL_STATUS), 0xff005184); // Is this needed twice?
}


void write_scaler_memory_registers(struct hdmi_information* hdmi_info) {

	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c008), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c00c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c010), 0x00000002);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c014), 0x00000002);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c018),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c01c),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c020),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1)));
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c024),
		    ((hdmi_info->timings.vactive.typ - 1) << 16 | (hdmi_info->timings.hactive.typ - 1))); //
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c028), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c02c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c030), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c034), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c038), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c03c), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c040), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c044), 0x00000000);
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c048), 0x00000000); //
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
	write_32bit_to_mem((uint32_t*)(dcss_base  + 0x1c180), 0x00040000); //
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