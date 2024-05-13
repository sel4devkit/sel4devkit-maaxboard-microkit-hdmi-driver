#include "dpr.h"
#include "write_register.h"
#include "dma.h"
#include "dma_offsets.h"

#include <stdio.h>
#include <stdlib.h>

void write_dpr_memory_registers(uintptr_t dcss_base, uintptr_t dma_base, struct hdmi_data *hdmi_config) {
	
	uintptr_t* fb_1 =  getPhys((void*) (dma_base));
	uintptr_t* fb_2 =  getPhys((void*) (dma_base + FRAME_BUFFER_TWO_OFFSET));

    write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1); // The address of the frame buffer
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)fb_1 + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	write_register((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.
	write_register((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	// write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004); // repeat
	//write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000013); // repeat + run + shadow enable

	uint32_t* dpr_sys_ctrl = (uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0);
	*dpr_sys_ctrl |= ((int)1 << 0);	// run enable
	// *dpr_sys_ctrl |= ((int)1 << 1);
	*dpr_sys_ctrl |= ((int)1 << 3); // shadow load en
	*dpr_sys_ctrl |= ((int)1 << 4); // shadow load select
	*dpr_sys_ctrl |= ((int)1 << 2);

	//*dpr_frame_ctrl |= ((int)1 << 2);
	//*dpr_frame_ctrl |= ((int)1 << 3); // rotate image by 180

	// These extra memory regsiters are written in case the 2nd channel needs to be used. (for using an additional alpha)

 	// write_register_debug((uint32_t*)(dcss_base + DPR_2_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_2); // The address of the frame buffer
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_CTRL0), 0x00000002); 
    // write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)fb_2 + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_PIX_X_CTRL), 0x00000280);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	// write_register((uint32_t*)(dcss_base + DPR_2_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

	// write_register((uint32_t*)(dcss_base + DPR_2_RTRAM_CTRL0), 0x00000038);
	// write_register((uint32_t*)(dcss_base + DPR_2_SYSTEM_CTRL0), 0x00000004);
	// write_register((uint32_t*)(dcss_base + DPR_2_SYSTEM_CTRL0), 0x00000005); // can this bit be set?
}

void write_dpr_memory_registers_two_channel(uintptr_t dcss_base, uintptr_t dma_base, struct hdmi_data *hdmi_config) {
	
	uintptr_t* fb_1 =  getPhys((void*) (dma_base));
	uintptr_t* fb_2 =  getPhys((void*) (dma_base + FRAME_BUFFER_TWO_OFFSET));

    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1); // The address of the frame buffer
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_CTRL0), 0x00000002); 
    write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)fb_1 + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_X_CTRL), 0x00000280);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	// write_register((uint32_t*)(dcss_base + DPR_1_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	write_register((uint32_t*)(dcss_base + DPR_1_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.
	//write_register((uint32_t*)(dcss_base + DPR_1_RTRAM_CTRL0), 0x00000038);
	// write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000004); // repeat
	//write_register((uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0), 0x00000013); // repeat + run + shadow enable

	uint32_t* dpr_sys_ctrl = (uint32_t*)(dcss_base + DPR_1_SYSTEM_CTRL0);
	*dpr_sys_ctrl |= ((int)1 << 0);	// run enable
	// *dpr_sys_ctrl |= ((int)1 << 1);
	*dpr_sys_ctrl |= ((int)1 << 3); // shadow load en
	*dpr_sys_ctrl |= ((int)1 << 4); // shadow load select
	*dpr_sys_ctrl |= ((int)1 << 2);

	//*dpr_frame_ctrl |= ((int)1 << 2);
	//*dpr_frame_ctrl |= ((int)1 << 3); // rotate image by 180

	// These extra memory regsiters are written in case the 2nd channel needs to be used. (for using an additional alpha)

 	write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_2); // The address of the frame buffer
	write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_CTRL0), 0x00000002); 
    write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_2_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)fb_1 + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_PIX_X_CTRL), 0x00000280);
	// write_register((uint32_t*)(dcss_base + DPR_2_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_register((uint32_t*)(dcss_base + DPR_2_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	write_register((uint32_t*)(dcss_base + DPR_2_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

	write_register((uint32_t*)(dcss_base + DPR_2_RTRAM_CTRL0), 0x00000038);
	// write_register((uint32_t*)(dcss_base + DPR_2_SYSTEM_CTRL0), 0x00000004);
	// write_register((uint32_t*)(dcss_base + DPR_2_SYSTEM_CTRL0), 0x00000005); // can this bit be set?


	uint32_t* dpr2_sys_ctrl = (uint32_t*)(dcss_base + DPR_2_SYSTEM_CTRL0);
	*dpr2_sys_ctrl |= ((int)1 << 0);	// run enable
	// *dpr_sys_ctrl |= ((int)1 << 1);
	*dpr2_sys_ctrl |= ((int)1 << 3); // shadow load en
	*dpr2_sys_ctrl |= ((int)1 << 4); // shadow load select
	*dpr2_sys_ctrl |= ((int)1 << 2);

	write_register((uint32_t*)(dcss_base + DPR_3_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1); // The address of the frame buffer
	write_register((uint32_t*)(dcss_base + DPR_3_FRAME_1P_CTRL0), 0x00000002); 
    write_register((uint32_t*)(dcss_base + DPR_3_FRAME_1P_PIX_X_CTRL), hdmi_config->H_ACTIVE);
	write_register((uint32_t*)(dcss_base + DPR_3_FRAME_1P_PIX_Y_CTRL), hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_3_FRAME_2P_BASE_ADDR_CTRL0), (uintptr_t)fb_1 + hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// write_register((uint32_t*)(dcss_base + DPR_3_FRAME_2P_PIX_X_CTRL), 0x00000280);
	// write_register((uint32_t*)(dcss_base + DPR_3_FRAME_2P_PIX_Y_CTRL), 0x000000f0);
	write_register((uint32_t*)(dcss_base + DPR_3_FRAME_CTRL0), ((hdmi_config->H_ACTIVE * 4) << 16));
	write_register((uint32_t*)(dcss_base + DPR_3_MODE_CTRL0), hdmi_config->rgb_format); // 32 bits per pixel (with rgba set to a certain value) This needs to be configured for differrent RGB ordering.

	//write_register((uint32_t*)(dcss_base + DPR_3_RTRAM_CTRL0), 0x00000038);
	// write_register((uint32_t*)(dcss_base + DPR_3_SYSTEM_CTRL0), 0x00000004);
	// write_register((uint32_t*)(dcss_base + DPR_3_SYSTEM_CTRL0), 0x00000005); // can this bit be set?


	uint32_t* dpr3_sys_ctrl = (uint32_t*)(dcss_base + DPR_3_SYSTEM_CTRL0);
	*dpr3_sys_ctrl |= ((int)1 << 0);	// run enable
	// *dpr_sys_ctrl |= ((int)1 << 1);
	*dpr3_sys_ctrl |= ((int)1 << 3); // shadow load en
	*dpr3_sys_ctrl |= ((int)1 << 4); // shadow load select
	*dpr3_sys_ctrl |= ((int)1 << 2);
}


void change_dpr_address(uintptr_t dcss_base, uintptr_t dma_base, struct hdmi_data *hdmi_config, int buffer) {
	uintptr_t* fb_1 =  getPhys((void*) (dma_base));
	uintptr_t* fb_2 =  getPhys((void*) (dma_base + FRAME_BUFFER_TWO_OFFSET));

	if (buffer == 0) {
		write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1); // The address of the frame buffer
	}
	else {
		write_register((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_2); // The address of the frame buffer
	}
}