#include <stdio.h>
#include <stdlib.h>

#include <microkit.h>


#include "dcss.h"
#include "dma.h"
#include "hdmi_data.h"
#include "dma_offsets.h"
#include "hdmi_tx.h"

#include "context_loader.h"
#include "dpr.h"
#include "dtg.h"
#include "scaler.h"
#include "sub_sampler.h"

#include "write_register.h"

#include "timer.h"

uintptr_t dcss_base;
uintptr_t dcss_blk_base;
uintptr_t gpc_base;
uintptr_t ccm_base;
uintptr_t dma_base;
uintptr_t dma_base_paddr;
uintptr_t timer_base;
uint32_t* active_frame_buffer_offset;
uint32_t* cache_frame_buffer_offset;

struct hdmi_data *hdmi_config = NULL;

// Included to stop error
// char *__heap_start;
// char *__heap_end;


void init(void) {
	
	initialise_and_start_timer(timer_base);
	sel4_dma_init(dma_base_paddr, dma_base, dma_base + DMA_SIZE);

	active_frame_buffer_offset = (uint32_t*)(dma_base + ACTIVE_FRAME_BUFFER_ADDR_OFFSET);	
	*active_frame_buffer_offset = FRAME_BUFFER_ONE_OFFSET; 

	cache_frame_buffer_offset = (uint32_t*)(dma_base + CACHE_FRAME_BUFFER_ADDR_OFFSET);	
	*cache_frame_buffer_offset = FRAME_BUFFER_TWO_OFFSET; 

	// malloc(5);

	init_gpc();
}

void
notified(microkit_channel ch) {

	switch (ch) {
		case 52:
			run_context_loader(dma_base, dcss_base, hdmi_config, active_frame_buffer_offset, cache_frame_buffer_offset);
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
	switch (ch) {
		case 0:
		    hdmi_config = (struct hdmi_data *) microkit_msginfo_get_label(msginfo);
			if (hdmi_config != NULL) {
				init_dcss();
			}
			else {
				printf("hdmi_data not configured properly in client PD\n");
			}
			return seL4_MessageInfo_new((uint64_t)hdmi_config,1,0,0); // what are the arguments?
			break;
		default:
			printf("Unexpected channel id: %d in dcss::protected() \n", ch);
	}
}

void init_dcss() {
	init_ccm();
	reset_dcss();
	init_hdmi(hdmi_config);
	write_dcss_memory_registers();

	if (hdmi_config->mode == MOVING_IMAGE) {
		init_context_loader(dma_base, dcss_base, hdmi_config, active_frame_buffer_offset, cache_frame_buffer_offset);
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

void write_dtrc_memory_registers() {
    
    write_register((uint32_t*)(dcss_base + DTCTRL_CHAN2), 0x2);
    write_register((uint32_t*)(dcss_base + DTCTRL_CHAN3), 0x2);
}

void write_dcss_memory_registers() {

	write_dtrc_memory_registers(dcss_base, hdmi_config);
	write_dpr_memory_registers(dcss_base, dma_base, hdmi_config);
	write_scaler_memory_registers(dcss_base, hdmi_config);
	write_sub_sampler_memory_registers(dcss_base, hdmi_config);
	write_dtg_memory_registers(dcss_base, hdmi_config);
}