#include "hdmi_tx.h"

#include <stdio.h>

// UBOOT
#include <inttypes.h>
#include <address.h>
#include <externs.h>
#include <API_HDMITX.h>
#include <source_phy.h> 
#include <API_AVI.h>
#include <API_AFE_t28hpc_hdmitx.h>

void init_hdmi(struct hdmi_data *hdmi_config) {
	
	uint8_t bits_per_pixel = 8; // 8 = 24 + alpha, 10= 30 +alpha
	VIC_PXL_ENCODING_FORMAT pixel_encoding_format = PXL_RGB;

	if (init_api() == CDN_OK) {
		uint32_t phy_frequency = phy_cfg_t28hpc(4, hdmi_config->PIXEL_FREQ_KHZ, bits_per_pixel, pixel_encoding_format, 1);
		hdmi_tx_t28hpc_power_config_seq(4);
		call_api(phy_frequency, pixel_encoding_format, bits_per_pixel, hdmi_config); // TODO: handle the return
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

void call_api(uint32_t phy_frequency, VIC_PXL_ENCODING_FORMAT pixel_encoding_format, uint8_t bits_per_pixel, struct hdmi_data *hdmi_config) {
	
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

	api_status = cdn_api_set_avi(hdmi_config, pixel_encoding_format, bt_type);
	print_api_status_msg(api_status, "cdn_api_set_avi");

	api_status = CDN_API_HDMITX_SetVic_blocking(hdmi_config, bits_per_pixel, pixel_encoding_format);
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