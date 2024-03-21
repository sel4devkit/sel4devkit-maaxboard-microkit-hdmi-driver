#ifndef __DCSS_H__
#define __DCSS_H__

#include <display_timing.h>
#include <vic_table.h>
#include <API_General.h>

// CCM
#define CCM_CCGR93_SET 0x45d4 // 5.1.7.7 CCM Clock Gating Register (CCM_CCGR93_SET)
#define CCM_TARGET_ROOT20 0x8a00 // 5.1.7.10 Target Register (CCM_TARGET_ROOT20)
#define CCM_TARGET_ROOT22 0x8b00 // 5.1.7.10 Target Register (CCM_TARGET_ROOT22)

// GPC
#define GPC_PGC_CPU_0_1_MAPPING 0x00EC // 5.2.10.31 PGC CPU mapping (GPC_PGC_CPU_0_1_MAPPING)
#define GPC_PU_PGC_SW_PUP_REQ 0x00F8 // 5.2.10.34 PU PGC software up trigger (GPC_PU_PGC_SW_PUP_REQ)

#define CONTROL0 0x10 // 15.2.2.1.6 Control (CONTROL0)

// DTRC
#define DTCTRL_CHAN2 0x160c8 // 15.6.6.1.28 DTRC Control (DTCTRL)
#define DTCTRL_CHAN3 0x170c8 // 15.6.6.1.28 DTRC Control (DTCTRL)

// DPR Channel 1
#define DPR_1_FRAME_1P_BASE_ADDR_CTRL0 0x180c0  // 15.7.3.1.38 Frame 1-Plane Base Address Control 0 (FRAME_1P_BASE_ADDR_CTRL0)
#define DPR_1_FRAME_1P_CTRL0 0x18090 // 15.7.3.1.26 Frame 1-Plane Control 0 (FRAME_1P_CTRL0)
#define DPR_1_FRAME_1P_PIX_X_CTRL 0x180a0 // 15.7.3.1.30 Frame 1-Plane Pix X Control (FRAME_1P_PIX_X_CTRL)
#define DPR_1_FRAME_1P_PIX_Y_CTRL 0x180b0 // 15.7.3.1.34 Frame 1-Plane Pix Y Control (FRAME_1P_PIX_Y_CTRL)
#define DPR_1_FRAME_2P_BASE_ADDR_CTRL0 0x18110 // 15.7.3.1.54 Frame 2-Plane Base Address Control 0 (FRAME_2P_BASE_ADDR_CTRL0)
#define DPR_1_FRAME_2P_PIX_X_CTRL 0x180f0 // 15.7.3.1.46 Frame 2-Plane Pix X Control (FRAME_2P_PIX_X_CTRL)
#define DPR_1_FRAME_2P_PIX_Y_CTRL 0x18100 // 15.7.3.1.50 Frame 2-Plane Pix Y Control (FRAME_2P_PIX_Y_CTRL)
#define DPR_1_FRAME_CTRL0 0x18070 // 15.7.3.1.22 Frame Control 0 (FRAME_CTRL0)
#define DPR_1_MODE_CTRL0 0x18050 // 15.7.3.1.18 Mode Control 0 (MODE_CTRL0)
#define DPR_1_RTRAM_CTRL0 0x18200 // 15.7.3.1.58 RTRAM Control 0 (RTRAM_CTRL0)
#define DPR_1_SYSTEM_CTRL0 0x18000 // 15.7.3.1.2 System Control 0 (SYSTEM_CTRL0)

// Color Sub-sampler
#define SS_COEFF 0x1b070 // 15.11.3.1.30 (SS_COEFF)
#define SS_CLIP_CB 0x1b080 //15.11.3.1.34 (SS_CLIP_CB)
#define SS_CLIP_CR 0x1b090 // 15.11.3.1.38 (SS_CLIP_CR)
#define SS_DISPLAY 0x1b010 // 15.11.3.1.6 (SS_DISPLAY)
#define SS_HSYNC 0x1b020 // 15.11.3.1.10 (SS_HSYNC)
#define SS_VSYNC 0x1b030 // 15.11.3.1.14 (SS_VSYNC)
#define SS_DE_ULC 0x1b040 // 15.11.3.1.18 (SS_DE_ULC)
#define SS_DE_LRC 0x1b050 // 15.11.3.1.22 (SS_DE_LRC)
#define SS_MODE 0x1b060 // 15.11.3.1.26 (SS_MODE)
#define SS_SYS_CTRL 0x1b000 // 15.11.3.1.2 (SS_SYS_CTRL)

// Display Timing Generator
#define TC_CONTROL_STATUS 0x20000 // 15.3.3.1.2 Timing Controller Control Register (TC_CONTROL_STATUS)
#define TC_DTG_REG1 0x20004 // 15.3.3.1.3 DTG lower right corner locations (TC_DTG_REG1)
#define TC_DISPLAY_REG2 0x20008 // 15.3.3.1.4 Display Register: TOP Window Coordinates for Active display area (TC_DISPLAY_REG2)
#define TC_DISPLAY_REG3 0x2000c // 15.3.3.1.5 Display Register: BOTTOM Window Coordinates for Activedisplay area (TC_DISPLAY_REG3)
#define TC_CH1_REG4 0x20010 // 15.3.3.1.6 Channel 1 window Register: TOP Window Coordinates forchannel1 (TC_CH1_REG4)
#define TC_CH1_REG5 0x20014 // 15.3.3.1.7 Channel_1 window Register: BOTTOM Window Coordinates forchannel_1 window (TC_CH1_REG5)
#define TC_CTX_LD_REG10 0x20028 // 15.3.3.1.12 Context Loader Register: Coordinates in the raster table wherethe context loader is started. (TC_CTX_LD_REG10)

// Scaler
#define SCALE_CTRL 0x1c000 // 15.8.3.1.2 Scale Control Register (SCALE_CTRL)
#define SCALE_OFIFO_CTRL 0x1c004 // 15.8.3.1.3 Scale Output FIFO Control Register (SCALE_OFIFO_CTRL)
#define SCALE_SRC_DATA_CTRL 0x1c008 // 15.8.3.1.4 Scale Source Data Control Register (SCALE_SRC_DATA_CTRL)
#define SCALE_BIT_DEPTH 0x1c00c //SCALE_BIT_DEPTH
#define SCALE_SRC_FORMAT 0x1c010 // 15.8.3.1.6 Scale Source Format Control Register (SCALE_SRC_FORMAT)
#define SCALE_DST_FORMAT 0x1c014 // 15.8.3.1.7 Scale Destination Format Control Register (SCALE_DST_FORMAT)
#define SCALE_SRC_LUMA_RES 0x1c018 // 15.8.3.1.8 Scale Source Luma Resolution Register (SCALE_SRC_LUMA_RES)
#define SCALE_SRC_CHROMA_RES 0x1c01c // 15.8.3.1.9 Scale Source Chroma Resolution Register (SCALE_SRC_CHROMA_RES)
#define SCALE_DST_LUMA_RES 0x1c020 // 15.8.3.1.10 Scale Destination Luma Resolution Register (SCALE_DST_LUMA_RES)
#define SCALE_DST_CHROMA_RES 0x1c024 // 15.8.3.1.11 Scale Destination Chroma Resolution Register (SCALE_DST_CHROMA_RES)
#define SCALE_V_LUMA_INC 0x1c04c // 15.8.3.1.13 Scale Vertical Luma Increment Register (SCALE_V_LUMA_INC)
#define SCALE_H_LUMA_INC 0x1c054 // 15.8.3.1.15 Scale Horizontal Luma Increment Register (SCALE_H_LUMA_INC)
#define SCALE_V_CHROMA_INC 0x1c05c // 15.8.3.1.17 Scale Vertical Chroma Increment Register (SCALE_V_CHROMA_INC)
#define SCALE_H_CHROMA_INC 0x1c064 // 15.8.3.1.19 Scale Horizontal Chroma Increment Register (SCALE_H_CHROMA_INC)

// #define SCALE_CTRL 0x1c0c0 // Need to locate in the spec
// #define SCALE_CTRL 0x1c140 
// #define SCALE_CTRL 0x1c180 
// #define SCALE_CTRL 0x1c1c0 

// Context loader
#define CTXLD_CTRL_STATUS 0x23000
#define DB_BASE_ADDR 0x23010
#define DB_COUNT 0x23014



void write_dcss_memory_registers();
void write_dtrc_memory_registers();
void write_dpr_memory_registers();
void write_sub_sampler_memory_registers();
void write_dtg_memory_registers();
void write_scaler_memory_registers();
void run_context_loader();
void init_context_loader();
void init_dcss();
void reset_dcss();
void init_gpc();
void init_ccm();
void init_hdmi();
CDN_API_STATUS init_api();
void call_api(uint32_t phy_frequency, VIC_PXL_ENCODING_FORMAT pixel_encoding_format, uint8_t bits_per_pixel);
void print_api_status_msg(CDN_API_STATUS status, char* function_name);

#endif