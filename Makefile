#
# Copyright 2021, Breakaway Consulting Pty. Ltd.
#
# SPDX-License-Identifier: BSD-2-Clause
#

# REQUIRED FOR FUNCTIONALITY
ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(MICROKIT_TOOL)),)
$(error MICROKIT_TOOL must be specified)
endif

ifeq ($(strip $(MICROKIT_BOARD)),)
$(error MICROKIT_BOARD must be specified)
endif

ifeq ($(strip $(MICROKIT_CONFIG)),)
$(error MICROKIT_CONFIG must be specified)
endif

# TOOLCHAIN := aarch64-none-elf
TOOLCHAIN := aarch64-linux-gnu

CPU := cortex-a53
CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as

# Define general warnings used by all protection domains
WARNINGS := -Wall -Wno-comment -Wno-return-type -Wno-unused-function -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-pointer-sign

# List of the object files needed for each protection domain
DCSS_OBJS 		:=  dcss.o dma.o picolibc_link.o vic_table.o API_general.o test_base_sw.o util.o API_AFE_t28hpc_hdmitx.o API_AFE.o vic_table.o API_HDMITX.o API_AVI.o API_Infoframe.o
CLIENT_OBJS		:=  example_client.o timer.o picolibc_link.o

# define c flags and includes for the dcss protection domain 
INC := $(BOARD_DIR)/include include include/hdmi
INC_PARAMS=$(foreach d, $(INC), -I$d)
CFLAGS := -mcpu=$(CPU) -mstrict-align  -nostdlib -nolibc -ffreestanding -g3 -O3 $(WARNINGS) $(INC_PARAMS) -I$(BOARD_DIR)/include --specs=picolibc/picolibc.specs -DSEL4 #-DSEL4_USB_DEBUG

# Define separate configuration for the client to avoid code bloat from unused includes
CLIENT_INC := $(BOARD_DIR)/include include/hdmi
CLIENT_INC_PARAMS=$(foreach d, $(INC), -I$d)
CLIENT_CFLAGS := -mcpu=$(CPU) -mstrict-align  -nostdlib -nolibc -ffreestanding -g3 -O3 $(WARNINGS) $(CLIENT_INC_PARAMS) -I$(BOARD_DIR)/include --specs=picolibc/picolibc.specs -DSEL4 #-DSEL4_USB_DEBUG

# Microkit lib flags
LDFLAGS := -L$(BOARD_DIR)/lib

# ideally we shouldn't need the -L path for libgcc
LIBS := -lmicrokit -Tmicrokit.ld -L/usr/lib/gcc-cross/aarch64-linux-gnu/10 -lgcc -Lpicolibc -lc  -L/usr/lib/gcc-cross/aarch64-linux-gnu/10 -lgcc 

# The images for each protetction domain
IMAGES := dcss.elf example-client.elf

# all target depends on the protection domain images to be built and the build_image target which builds the final image 
all: $(addprefix $(BUILD_DIR)/, $(IMAGES)) build_image

# Compile the files in the src directory
$(BUILD_DIR)/%.o: src/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Compile the files in the hdmi directory
$(BUILD_DIR)/%.o: src/hdmi/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Compile the example client file 
$(BUILD_DIR)/%.o: src/example-client/%.c Makefile
	$(CC) -c $(CLIENT_CFLAGS) $< -o $@

# Compile the object file for picolibc printf to work
$(BUILD_DIR)/%.o: picolibc/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Create elf files for DCSS protection domain
$(BUILD_DIR)/dcss.elf: $(addprefix $(BUILD_DIR)/, $(DCSS_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

# Create elf files for DCSS protection domain
$(BUILD_DIR)/example-client.elf: $(addprefix $(BUILD_DIR)/, $(CLIENT_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

# define the main image file and the report file
IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

# build entire system
build_image: $(IMAGE_FILE) 
$(IMAGE_FILE) $(REPORT_FILE): sel4-hdmi.system 
	$(MICROKIT_TOOL) sel4-hdmi.system --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

# (optional) clean
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/.depend*
	find . -name \*.o |xargs --no-run-if-empty rm
