// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 *
 */

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

#define FRAME_BUFFER_SIZE 1920 * 1080 * 4 // re define 
#define VIC_MODE 1 // Understand different vic modes

void init(void) {
    
	printf("Init DCSS\n");
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