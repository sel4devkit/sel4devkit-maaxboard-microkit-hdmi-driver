
#include "dma_offsets.h"
#include "double_buffer.h"
#include "dma.h"
#include "timer.h"
#include "hdmi_data.h"
#include "write_register.h"

#include "dpr.h"
#include "dtg.h"
#include "scaler.h"

#include <microkit.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define CTXLD_CTRL_STATUS 0x23000
#define DB_BASE_ADDR 0x23010
#define DB_COUNT 0x23014


int context = 0; // This keeps track of the current context. TODO: An alternative to global counter?

void init_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* active_frame_buffer_offset, uint32_t* cache_frame_buffer_offset) {

	// Steps 1 and 2 of 15.4.2.2 Display state loading sequence are done here as the double buffered registers do not need to change what they contain.
	// So it should only be written once.
    
	uintptr_t* frame_buffer1_addr = getPhys((void*)dma_base);                           // get do this before and pass it in.
	uintptr_t* frame_buffer2_addr = getPhys((void*)dma_base + FRAME_BUFFER_TWO_OFFSET);

	/*  
		The context loader has access to two double buffered registers depening on the current context.
		These registers are 64 bit and hold the address of the frame buffer in the first 32 bits and
		the DPR memory register where the frame buffer will be set in the second 32 bits. 
		See 15.4.2.3 System Memory Display state format
	*/
	uint32_t* ctx_ld_db1_addr = (uint32_t*)(dma_base + CTX_LD_CTX_LDE_ADDR); 	
	*ctx_ld_db1_addr = (uintptr_t)frame_buffer1_addr;							
	ctx_ld_db1_addr++; 															
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 
	
	// This extra memory register has been added as the value is based on the current frame buffer (it seems to have no effect in context switching)
	ctx_ld_db1_addr++; 
	*ctx_ld_db1_addr = (uintptr_t)frame_buffer1_addr + (hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	ctx_ld_db1_addr++; 
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0; 


	uint32_t* ctx_ld_db2_addr = (uint32_t*)(dma_base + CTX_LD_DB_TWO_ADDR); 	
	*ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr;							
	ctx_ld_db2_addr++; 															
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; 

	// This extra memory register has been added as the value is based on the current frame buffer (it seems to have no effect in context switching)
	ctx_ld_db2_addr++; 
	*ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr + (hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	ctx_ld_db2_addr++; 
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_2P_BASE_ADDR_CTRL0; 


	// uint32_t* ctx_ld_db2_addr = (uint32_t*)(dma_base + CTX_LD_DB_TWO_ADDR); 	
	// *ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr;							
	// ctx_ld_db2_addr++; 															
	// *ctx_ld_db2_addr = dcss_base + DPR_2_FRAME_1P_BASE_ADDR_CTRL0; 

	// // This extra memory register has been added as the value is based on the current frame buffer
	// ctx_ld_db2_addr++; 
	// *ctx_ld_db2_addr = (uintptr_t)frame_buffer2_addr + (hdmi_config->H_ACTIVE * hdmi_config->V_ACTIVE);
	// ctx_ld_db2_addr++; 
	// *ctx_ld_db2_addr = dcss_base + DPR_2_FRAME_2P_BASE_ADDR_CTRL0; 
	
	run_context_loader(dma_base, dcss_base, hdmi_config, active_frame_buffer_offset, cache_frame_buffer_offset);
}


void run_context_loader(uintptr_t dma_base, uintptr_t dcss_base, struct hdmi_data *hdmi_config, uint32_t* active_frame_buffer_offset, uint32_t* cache_frame_buffer_offset){
	
	// Steps 3,4,5 and 12 of 15.4.2.2 Display state loading sequence

	printf("Running context loader in context: %d\n", context);
	start_timer();
	uint32_t* enable_status = (uint32_t*)(dcss_base + CTXLD_CTRL_STATUS);
	uint32_t* dpr_sys_ctrl = (uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0);
	uint32_t* scaler_sys_ctrl = (uint32_t*)(dcss_base + SCALE_CTRL);
	int context_ld_enabled = 0;

	uint32_t* int_status = (uint32_t*)(dcss_base + TC_INTERRUPT_STATUS);
	uint32_t* int_control = (uint32_t*)(dcss_base + TC_INTERRUPT_CONTROL_REG17);
	uint32_t* int_mask = (uint32_t*)(dcss_base + TC_INTERRUPT_MASK);
	
	// *dpr_sys_ctrl |= ((int)1 << 0);	// does bad things (pre loads the buffer)
	// *dpr_sys_ctrl |= ((int)1 << 2);
	// *dpr_sys_ctrl |= ((int)1 << 3); // shadow load en
	// *dpr_sys_ctrl |= ((int)1 << 4);


	*scaler_sys_ctrl |= ((int)1 << 0);
	// *scaler_sys_ctrl |= ((int)1 << 4);

	// Give priority to the context loader TODO: Probably only needs to be done once per initialisation
	*enable_status |= ((int)1 << 1);

	// Set the context offset in memory for the current frame buffer to display
	int contex_offset = (context == 0) ? CTX_LD_CTX_LDE_ADDR : CTX_LD_DB_TWO_ADDR;		


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
	write_register((uint32_t*)(dcss_base + DB_COUNT), 2);


	////////////////////////////////////////////////////////////////////////
	// *int_control |= ((int)1 << 1); // clear line 1 so that it does not show as asserted
	// *int_mask |= ((int)1 << 1); // enable interrupt for line 1 ()
	// int line1_status = (*int_status >> 1) & (int)1; // check status 

	// while (line1_status == 0) {	
	// 	line1_status = (*int_status >> 1) & (int)1;
	// 	seL4_Yield();
	// 	//printf("still here\n");	// having a print statement makes it an unpredictable amount of time.
	// }
	////////////////////////////////////////////////////////////////////////					
	
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
	*active_frame_buffer_offset = (context == 0) ? FRAME_BUFFER_TWO_OFFSET : FRAME_BUFFER_ONE_OFFSET;
	*cache_frame_buffer_offset = (context == 0) ?  FRAME_BUFFER_ONE_OFFSET : FRAME_BUFFER_TWO_OFFSET;
	context = context == 1 ? 0 : 1; 																			
	printf("Switching context took %d ms\n", stop_timer());

	//ms_delay(1000); // For debugging 
	if (hdmi_config->ms_delay != NO_DELAY) {
		ms_delay(hdmi_config->ms_delay);
	}

	// Notify the client to draw the frame buffer
	microkit_notify(52);
}

/* 	
	Testing using interupts rather than the context loader
	Using the interrupts doesn't seem to have an affect on the point at which it is drawn.
*/
// void change_buffer() {

// 	ms_delay(3000); // let the screen turn on and the init_dcss() to take affect.

// 	//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x1E000000); // 480
// 	//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0xAA000000); //680
// 	//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x89800000); // 1100
// 	//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0);
	
// 	uintptr_t* fb_1 =  getPhys((void*) (dma_base));
// 	uintptr_t* fb_2 =  getPhys((void*) (dma_base + FRAME_BUFFER_TWO_OFFSET));

// 	uint32_t* line1_intr = (uint32_t*)(dcss_base + TC_LINE1_INT_REG13);
// 	uint32_t* int_status = (uint32_t*)(dcss_base + TC_INTERRUPT_STATUS);
// 	uint32_t* int_control = (uint32_t*)(dcss_base + TC_INTERRUPT_CONTROL_REG17);
// 	uint32_t* int_mask = (uint32_t*)(dcss_base + TC_INTERRUPT_MASK);
	
// 	write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x11300000); // 1100
// 	write_register((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0); // 1100

// 	*line1_intr |= ((int)1 << 25);
// 	*line1_intr |= ((int)1 << 26);
// 	*line1_intr |= ((int)1 << 27);
// 	*line1_intr |= ((int)1 << 28);


// 	ms_delay(1000);

// 	for (size_t i = 0; i < 10000; i++){
		
// 		//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x1E000000); // if i reset it, its different each time
// 		// write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x89800000); // 1100 this appears higher than 0
// 		//write_register_debug((uint32_t*)(dcss_base + TC_LINE1_INT_REG13), 0x11300000); // 1100
// 		*int_control |= ((int)1 << 1); // clear line 1 so that it does not show as asserted
		
// 		// Writing the binary value in the register
// 		*line1_intr |= ((int)1 << 16); // no matter how i set these bits, it still changes at the same interval.
// 		*line1_intr |= ((int)1 << 17);
// 		*line1_intr |= ((int)1 << 18);
// 		*line1_intr |= ((int)1 << 19);
// 		*line1_intr |= ((int)1 << 20);
// 		*line1_intr |= ((int)1 << 21);
// 		*line1_intr |= ((int)1 << 22);
// 		*line1_intr |= ((int)1 << 23);
// 		*line1_intr |= ((int)1 << 24);
// 		*line1_intr |= ((int)1 << 25);
// 		*line1_intr |= ((int)1 << 26);
// 		*line1_intr |= ((int)1 << 27);
// 		*line1_intr |= ((int)1 << 28);
		
// 		*int_control |= ((int)1 << 1); // clear line 1 so that it does not show as asserted
// 		*int_mask |= ((int)1 << 1); // enable interrupt for line 1 ()
// 		int line1_status = (*int_status >> 1) & (int)1; // check status 

// 		while (line1_status == 0) {	
// 			line1_status = (*int_status >> 1) & (int)1;
// 			seL4_Yield();
// 			//printf("still here\n");	// having a print statement makes it an unpredictable amount of time.
// 		}
		
// 		ms_delay(12); // depending on this delay it will draw it at different points.
// 		if (i % 2 == 0) {
// 			write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_2);
// 		}
// 		else {
// 			write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1);
// 		}
// 		ms_delay(1500);
// 		//printf("int status after = %d\n", line1_status);
// 	}
// }

// /* 	
// 	Testing changing buffer manually at differnt time intervals
// */
// void change_buffer_manually() {

// 	// This test shows that its changing it without any regard to the DTG
// 	// I can see it change randomly in the middle of the screen
// 	// In the current example it changes from the top everytime
// 	// I am quite certain that it is doing this during the active display time, rather than vertical blanking time.
	
// 	ms_delay(5000);
// 	int delays[10] = {5,18,19,20,100,333,666,1000,1111,1234};

// 	uintptr_t* fb_1 =  getPhys((void*) (dma_base));
// 	uintptr_t* fb_2 =  getPhys((void*) (dma_base + FRAME_BUFFER_TWO_OFFSET));

// 	for (size_t i = 0; i < 10; i++)
// 	{
// 		if (i % 2 == 0) {
// 			write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_2);
// 		}
// 		else {
// 			write_register_debug((uint32_t*)(dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0), (uintptr_t)fb_1);
// 		}
// 		ms_delay(delays[i]);
// 	}
	

// 	// here to test api example 5:
// 	ms_delay(3000);
// 	uint32_t* dpr_sys_ctrl = (uint32_t*)(dcss_base + DPR_2_FRAME_1P_BASE_ADDR_CTRL0);

// 	*dpr_sys_ctrl |= ((int)1 << 0);	// does bad things
// 	*dpr_sys_ctrl |= ((int)1 << 2);
// 	*dpr_sys_ctrl |= ((int)1 << 3); // shadow load en
// 	*dpr_sys_ctrl |= ((int)1 << 4);
// }