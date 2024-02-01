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
#define VIC_MODE 1 // try a different vic mode

void init(void) {
    sel4_dma_init(frame_buffer_paddr, frame_buffer, frame_buffer + FRAME_BUFFER_SIZE); // init frame buffer
    hdmi_set_timings(&hdmi_info);
	init_dcss();
	init_hdmi();
    write_dcss_memory_registers(&hdmi_info);
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
	hdmi_info->timings.pixelclock.typ = pixel_clock_frequency * 1000; // use a define
}


void init_dcss(){

	write_32bit_to_mem((uint32_t*)(ccm_base + 0x45d4), 0x3); 		// 5.1.7.7 CCM Clock Gating Register (CCM_CCGR93_SET)
	write_32bit_to_mem((uint32_t*)(gpc_base + 0x00EC), 0xffff); 	// 5.2.10.31 PGC CPU mapping (GPC_PGC_CPU_0_1_MAPPING)
	write_32bit_to_mem((uint32_t*)(gpc_base + 0x00F8), 0x1 << 10);  // 5.2.10.34 PU PGC software up trigger (GPC_PU_PGC_SW_PUP_REQ) // Does the bit shifting here matter - should it just be hardcoded?

	write_32bit_to_mem((uint32_t*)(ccm_base + 0x8a00), 0x12000000); // 5.1.7.10 Target Register (CCM_TARGET_ROOT20)
	write_32bit_to_mem((uint32_t*)(ccm_base + 0x8b00), 0x11010000); // 5.1.7.10 Target Register (CCM_TARGET_ROOT22)

	write_32bit_to_mem((uint32_t*)(dcss_blk_base), 0xffffffff); // 15.2.2.1.2 Reset Control (RESET_CTRL)
	write_32bit_to_mem((uint32_t*)(dcss_blk_base + 0x10), 0x1); // 15.2.2.1.6 Control (CONTROL0)
}


void init_hdmi() {
	uint8_t bits_per_pixel = 8;
	VIC_MODES vic_mode = 1;
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = 1;
	uint32_t character_freq_khz = phy_cfg_t28hpc(4, vic_mode, bits_per_pixel, pixel_encoding_format, 1);
	hdmi_tx_t28hpc_power_config_seq(4);
}


void write_dcss_memory_registers(struct hdmi_information* hdmi_info){


}