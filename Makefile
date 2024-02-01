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

DCSS_OBJS 		:=  dcss.o dma.o picolibc_link.o vic_table.o API_general.o test_base_sw.o util.o API_AFE_t28hpc_hdmitx.o API_AFE.o vic_table.o
CLIENT_OBJS		:=  example_client.o dma.o picolibc_link.o

INC := $(BOARD_DIR)/include include include/hdmi
INC_PARAMS=$(foreach d, $(INC), -I$d)
WARNINGS := -Wall -Wno-comment -Wno-return-type -Wno-unused-function -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label -Wno-pointer-sign
CFLAGS := -mcpu=$(CPU) -mstrict-align  -nostdlib -nolibc -ffreestanding -g3 -O3 $(WARNINGS) $(INC_PARAMS) -I$(BOARD_DIR)/include --specs=picolibc/picolibc.specs -DSEL4 #-DSEL4_USB_DEBUG
LDFLAGS := -L$(BOARD_DIR)/lib

# redo the client INC, no need to bloat the binary with things it doesn't need


# ideally we shouldn't need the -L path for libgcc
LIBS := -lmicrokit -Tmicrokit.ld -L/usr/lib/gcc-cross/aarch64-linux-gnu/10 -lgcc -Lpicolibc -lc  -L/usr/lib/gcc-cross/aarch64-linux-gnu/10 -lgcc 


# forcing it to build, this will need to change
# all: $(BUILD_DIR)/dcss.o $(BUILD_DIR)/dma.o $(BUILD_DIR)/picolibc_link.o $(BUILD_DIR)/API_general.o $(BUILD_DIR)/test_base_sw.o $(BUILD_DIR)/util.o $(BUILD_DIR)/API_AFE_t28hpc_hdmitx.o $(BUILD_DIR)/API_AFE.o $(BUILD_DIR)/vic_table.o $(BUILD_DIR)/API_HDMITX.o $(BUILD_DIR)/API_AVI.o $(BUILD_DIR)/API_Infoframe.o $(BUILD_DIR)/dcss.elf 
all: $(BUILD_DIR)/dcss.o $(BUILD_DIR)/dma.o $(BUILD_DIR)/picolibc_link.o $(BUILD_DIR)/API_general.o $(BUILD_DIR)/test_base_sw.o $(BUILD_DIR)/util.o $(BUILD_DIR)/API_AFE_t28hpc_hdmitx.o $(BUILD_DIR)/API_AFE.o $(BUILD_DIR)/vic_table.o $(BUILD_DIR)/example_client.o $(BUILD_DIR)/example-client.elf  $(BUILD_DIR)/dcss.elf

# # Compile each of the object files for DCSS and HDMI protection domains
$(BUILD_DIR)/%.o: src/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: src/hdmi/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@


$(BUILD_DIR)/%.o: src/example-client/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@


# Compile the object file for picolibc printf to work
$(BUILD_DIR)/picolibc_link.o: picolibc/picolibc_link.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

# Create elf files for DCSS protection domain
$(BUILD_DIR)/dcss.elf: $(addprefix $(BUILD_DIR)/, $(DCSS_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

# Create elf files for DCSS protection domain
$(BUILD_DIR)/example-client.elf: $(addprefix $(BUILD_DIR)/, $(CLIENT_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@


# Build complete system
IMAGE_FILE = $(BUILD_DIR)/loader.img
REPORT_FILE = $(BUILD_DIR)/report.txt

all: $(IMAGE_FILE)

# add elf file to list of images
# Use $(API_IMAGES) to reference driver required elfs (DCSS and HDMI for the time being)
# IMAGES := hdmi.elf dcss.elf example_client.elf
IMAGES := dcss.elf  example-client.elf

# build entire system
$(IMAGE_FILE) $(REPORT_FILE): $(addprefix $(BUILD_DIR)/, $(IMAGES)) sel4-hdmi.system
	$(MICROKIT_TOOL) sel4-hdmi.system --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

# (optional) clean
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf $(BUILD_DIR)/.depend*
	find . -name \*.o |xargs --no-run-if-empty rm
