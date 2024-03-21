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
uintptr_t rc_base;
uintptr_t dma_base;
uintptr_t dma_base_paddr;
uintptr_t frame_buffer1_start_addr;
uintptr_t frame_buffer2_start_addr;

uintptr_t timer_base;

// uintptr_t ctx_ld_db1;
// uintptr_t ctx_ld_db2;
// uintptr_t ctx_ld_db1_paddr;
// uintptr_t ctx_ld_db2_paddr;

struct hdmi_data *v_data = NULL;

// #define FRAME_BUFFER_SIZE 1280 * 720 * 3 // TODO: re define  
// #define CTX_LD_DMA_SIZE 0x100000 // Have a proper think about this size (how big does it realisticly need to be, probably no way near as large as this)

void run_context_loader();


// I could potentially have a frame buffer PD
// This will need to keep track of whats written so it can clear it easily. (I can try clearing the whole thing first of all to be easier)
// This frame buffer PD will manage the clearing and the writing. The user will simply just write their frame buffer (if its moving) to one place
// So if the user is writing to it directly, then in the example client PD it must be pointing to either one of the frame buffers at any given time.
// So if there was a frame buffer PD, this would potentially be responsible for switching what the pointer in the client points to.



// void write_frame_buffer1(int width, int height, int offset); // temporary
// void write_frame_buffer2(int width, int height, int offset); // temporary

// void clear_frame_buffer1(int width, int height, int offset);
// void clear_frame_buffer2(int width, int height, int offset);


// void write_frame_buffer1(int width, int height, int offset) {

// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer1_start_addr;
// 	int side_length = height;
// 	//int side_length = 20;
// 	int side_width = 80;

// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0xff;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0xff;
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}

// 	// for (int i = 0; i < side_length; i++) {
// 	// 	for (int j = 0; j < side_length; j++) {
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 	}
// 	// 	frame_buffer_addr += 4*(width-side_length);
// 	// }
// }

// // // two functions are not needed, but just here for testing purposes to reduce chances of error using pointers.
// void write_frame_buffer2(int width, int height, int offset) {

// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer2_start_addr;
// 	int side_length = height;
// 	//int side_length = 20;
// 	int side_width = 80;
	
// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0xff;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0xff;
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}

// 	// for (int i = 0; i < side_length; i++) {
// 	// 	for (int j = 0; j < side_length; j++) {
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 	}
// 	// 	frame_buffer_addr += 4*(width-side_length);
// 	// }
// }


// void clear_frame_buffer1(int width, int height, int offset) { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled. 
	
// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer1_start_addr;
// 	int side_length = height;
// 	int side_width = 80;

// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}

// 	// for (int i = 0; i < side_length; i++) {
// 	// 	for (int j = 0; j < side_length; j++) {
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 	}
// 	// 	frame_buffer_addr += 4*(width-side_length);
// 	// }
// }


// void clear_frame_buffer2(int width, int height, int offset) { // TODO: We may not always want to clear the entire frame buffer if only part of it is filled. 
	
// 	uint8_t* frame_buffer_addr = (uint8_t*)frame_buffer2_start_addr;
// 	int side_length = height;
// 	int side_width = 80;

// 	for (int i = 0; i < side_length; i++) {
// 		frame_buffer_addr += 4*(offset);
// 		for (int j = 0; j < side_width; j++) {
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 			*(frame_buffer_addr++) = 0x00;
// 		}
// 		frame_buffer_addr += 4*(width-side_width-offset);
// 	}

// 	// for (int i = 0; i < side_length; i++) {
// 	// 	for (int j = 0; j < side_length; j++) {
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0x00;
// 	// 		*(frame_buffer_addr++) = 0xff;
// 	// 	}
// 	// 	frame_buffer_addr += 4*(width-side_length);
// 	// }
// }


void init(void) {
	
	printf("Init Dcss\n");
	initialise_and_start_timer(timer_base);
	sel4_dma_init(dma_base_paddr, dma_base, dma_base + CTX_LD_DMA_SIZE); // This is too big and needs to be thought of more carefully.
	init_gpc();

		// ms_delay(1000);




// This is the only initialisation needed ^^^^^^

// This can go once the current api examples have been set up to work across channels

	// test it works without context switching first.
	// v_data = malloc(sizeof(struct hdmi_data));
	// struct hdmi_data v = {1650, 1280, 370, 40, 110, 220, 750, 720, 5, 5, 20, 74250, 1, 1, 8, 0, 23, GBRA, ALPHA_ON};
	// *v_data = v;

	int i = malloc(sizeof(int)); // hack not needed
	// // to test the context loader load dcss from here and comment out code in example client.
	// init_dcss();

	// frame_buffer1_start_addr = dma_base;
	// frame_buffer2_start_addr = dma_base + (FRAME_BUFFER_SIZE * 3); // have a better way of doing this, the sizing should be reconsidered

	// write_frame_buffer1(v_data->H_ACTIVE, v_data->V_ACTIVE, 10);

	// double_buffer_test();

	// // frame_buffer1_start_addr = frame_buffer1;
	// // frame_buffer2_start_addr = frame_buffer2;


	// printf("Should be calling notify\n");
	// microkit_notify(52);
	// printf("end of calling notify\n");
}

void run_context_loader(){

	int context = 0;
	int i = 0;

	// This whole thing can be properly tidied up and neatly in a structure or something.

	// Tell the two shadow registers (for each context) where the DPR address is
	// The first 32 bits is the frame buffer address itself and the next 32 bits is the memory register
	// Then in the contex loader i need to say where the base of this DMA memory is

	uintptr_t* frame_buffer1_addr = getPhys((void*)dma_base);
	uintptr_t* frame_buffer2_addr = getPhys((void*)dma_base + FRAME_BUFFER_TWO_OFFSET);

	printf("Frame buffer 1 addr = %x\n", (uintptr_t)frame_buffer1_addr);
	printf("Frame buffer 2 addr = %x\n", (uintptr_t)frame_buffer2_addr);
	printf("Frame buffer 1 addr pointer = %p\n", frame_buffer1_addr);
	printf("Frame buffer 2 addr pointer = %p\n", frame_buffer2_addr);

	uint32_t* ctx_ld_db1_addr = (uint32_t*)(dma_base + (FRAME_BUFFER_SIZE * 2)); // This needs to be an offset from the dma base (frame buffer size *2)
	printf("Context load adress addr = %p\n", ctx_ld_db1_addr);
	
	*ctx_ld_db1_addr = (uint32_t)frame_buffer1_addr;				// Set the address of the first frame buffer
	printf("Context load value  = %x\n", *ctx_ld_db1_addr);

	ctx_ld_db1_addr++; // should move it along by 32 bits
	*ctx_ld_db1_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; // The memory register that we are changing (the DPR Address)
	printf("Context load adress 2 addr = %p\n", ctx_ld_db1_addr);
	printf("Context load value 2 = %x\n", *ctx_ld_db1_addr);
	ctx_ld_db1_addr--;												// To use the physical one we can just use getPhys() so decrementing it back shouldn't need to be a thing
	printf("Context load adress 3 addr = %p\n", ctx_ld_db1_addr);
	printf("Context load value 3 = %x\n", *ctx_ld_db1_addr);

	uint32_t* ctx_ld_db2_addr = (uint32_t*)(dma_base + (FRAME_BUFFER_SIZE * 2) + 1000000); // This needs to be an offset from the dma base (frame buffer size *2) + something
	printf("2Context load adress addr = %p\n", ctx_ld_db2_addr);
	
	*ctx_ld_db2_addr = (uint32_t)frame_buffer2_addr;				// Set the address of the second frame buffer
	printf("2Context load value  = %x\n", *ctx_ld_db2_addr);

	ctx_ld_db2_addr++; // should move it along by 32 bits
	*ctx_ld_db2_addr = dcss_base + DPR_1_FRAME_1P_BASE_ADDR_CTRL0; // The memory register that we are changing (the DPR Address)
	printf("2Context load adress 2 addr = %p\n", ctx_ld_db2_addr);
	printf("2Context load value 2 = %x\n", *ctx_ld_db2_addr);
	ctx_ld_db2_addr--;												// To use the physical one we can just use getPhys() so decrementing it back shouldn't need to be a thing
	printf("2Context load adress 3 addr = %p\n", ctx_ld_db2_addr);
	printf("2Context load value 3 = %x\n", *ctx_ld_db2_addr);

	// All of the above code can go in a separate initilsation function done just once.

	uint32_t* enable_status = (uint32_t*)(dcss_base + CTXLD_CTRL_STATUS);
	int context_ld_enabled = 0;
	int db_load_sequence_complete = 0;
	int sb_lp_load_sequence_complete = 0;
	int sb_hp_load_sequence_complete = 0;

	*enable_status |= ((int)1 << 1); // set the enable status bit to 1 to kickstart process.

	//int count = 10;
	while (i == 0) { // don't have this go forever in future - I should call this loop at an interval that i want it to draw a new image at. This will just flash very quickly

		int arb_sel = (*enable_status >> 1) & (int)1;
		printf("ARB sel = %d\n", arb_sel);

		// printf("enable status = %d\n", context_ld_enabled);			// this isn't the nicest code (or i could instead change it to ctx_ld enabled )
		// while (context_ld_enabled == 1) {
		// 	context_ld_enabled = (*enable_status >> 0) & (int)1;
		// 	printf("enable status = %d\n", context_ld_enabled);		
		// }

		if (context == 0) {
			// if our context is 0 make sure that it is currently looking at the first frame buffer
			printf("Setting the current context to use the first frame buffer.\n");
			//write_32bit_to_mem((uint32_t*)(dcss_base + DB_BASE_ADDR), getPhys((void*)dma_base + ((FRAME_BUFFER_SIZE * 2)+1000000)));
			write_32bit_to_mem((uint32_t*)(dcss_base + DB_BASE_ADDR), getPhys((void*)dma_base + ((FRAME_BUFFER_SIZE * 2))));
		}
		else {	
			printf("Setting the current context to use the second frame buffer.\n");
			write_32bit_to_mem((uint32_t*)(dcss_base + DB_BASE_ADDR), getPhys((void*)dma_base + ((FRAME_BUFFER_SIZE * 2)+1000000)));
			//write_32bit_to_mem((uint32_t*)(dcss_base + DB_BASE_ADDR), getPhys((void*)dma_base + ((FRAME_BUFFER_SIZE * 2)+1000000)));
		}

		write_32bit_to_mem((uint32_t*)(dcss_base + DB_COUNT), 1);



		db_load_sequence_complete = (*enable_status >> 17) & (int)1;
		printf("db loading sequence default = %d\n", db_load_sequence_complete);		
		
		printf("enable status = %x\n", *enable_status);
		context_ld_enabled = (*enable_status >> 0) & (int)1; // if that bit is set then it is 1
		printf("enable status check = %d\n", context_ld_enabled);
		
		*enable_status |= ((int)1 << 0); // set the enable status bit to 1 to kickstart process.
		
		printf("enable status after = %x\n", *enable_status);
		context_ld_enabled = (*enable_status >> 0) & (int)1; // if that bit is set then it is 1
		printf("enable status check after = %d\n", context_ld_enabled);


		// in this time now that it is enabled and writing a context. It should write the other contexts frame buffer
		// It is important that there is a slight delay here so it can clear it before it starts doing things. (This feels a little bit hacky, in the spec it says wait until its not busy)
		
		// db_load_sequence_complete = (*enable_status >> 17) & (int)1;
		// sb_hp_load_sequence_complete = (*enable_status >> 18) & (int)1;
		// sb_lp_load_sequence_complete = (*enable_status >> 19) & (int)1;
		// while (db_load_sequence_complete == 0) {
		// 	  // sb_hp_load_sequence_complete == 0 ||
		// 	 //  sb_lp_load_sequence_complete == 0) {
			
		// 	db_load_sequence_complete = (*enable_status >> 17) & (int)1;
		// 	//sb_hp_load_sequence_complete = (*enable_status >> 18) & (int)1;
		// 	//sb_lp_load_sequence_complete = (*enable_status >> 19) & (int)1;
		// 	printf("db loading sequence active = %d\n", db_load_sequence_complete);	
		// 	//printf("sb lp loading sequence active = %d\n", sb_lp_load_sequence_complete);	
		// 	//printf("sb hp loading sequence active = %d\n", sb_hp_load_sequence_complete);	
		// }


		// While this is happening i shouldn't alter either frame buffer. (I think... )
		while (context_ld_enabled == 1) {
			context_ld_enabled = (*enable_status >> 0) & (int)1;
			printf("enable status second time = %d\n", context_ld_enabled);		
		}

		printf("db loading sequence complete = %d\n", db_load_sequence_complete);	


		// ms_delay(1000);
		printf("Should be calling notify\n");
		microkit_notify(52);

		ms_delay(1000);
		printf("end of calling notify\n");

		// It shouldn't matter what the context is - it will just tell it to write the frame buffer
		// The other PD will just switch and use the other buffer that it was using.
		// It could just increment a counter in an array to do this

	//	ms_delay(8000);

		

		// could use something a little more sophisticated 
		context = context == 1 ? 0 : 1;
		//count +=1;
	}
}




		// int b = 6;	//10011
		// // check the the bits to see if they are what they say they are

		// int check = (b >> 0) & (int)1;
		// printf("bit 1 set = %d\n", check);
		// check = (b >> 1) & (int)1;
		// printf("bit 2 set = %d\n", check);
		// check = (b >> 2) & (int)1;
		// printf("bit 3 set = %d\n", check);
		// check = (b >> 3) & (int)1;
		// printf("bit 4 set = %d\n", check);


		// // Toggle a bit to 1 so the number changes
		// b |= ((int)1 << 3);
		// printf("b after = %d\n", b);

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
		printf("v data not yet set\n"); // This should then not go try init things, handle case properly 
	}

	init_ccm();
	reset_dcss();
	init_hdmi();
    write_dcss_memory_registers();



	// This wont happen here - instead it will be called from the client depending on the db_toggle in v_data
	// A function pointer to the write frame buffer call would be useful so it knows which frame buffer example to use.
	if (v_data->db_toggle) {
		run_context_loader();
	}
	// microkit_notify(46);
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