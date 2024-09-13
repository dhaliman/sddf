#
# Copyright 2024, UNSW
#
# SPDX-License-Identifier: BSD-2-Clause
#

ifeq ($(strip $(MICROKIT_SDK)),)
$(error MICROKIT_SDK must be specified)
endif

ifeq ($(strip $(SDDF)),)
$(error SDDF must be specified)
endif

ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(MICROKIT_CONFIG)),)
$(error MICROKIT_CONFIG must be specified)
endif

ifeq ($(strip $(FB_IMG)),)
$(error FB_IMG must be specified)
endif

ifeq ($(strip $(MICROKIT_BOARD)), qemu_virt_aarch64)
	TIMER_DRIVER_DIR := arm
	GPU_DRIVER_DIR := virtio
	CPU := cortex-a53
else
$(error Unsupported MICROKIT_BOARD given)
endif

ifeq ($(strip $(TOOLCHAIN)),)
	TOOLCHAIN := clang
endif

ifeq ($(strip $(TOOLCHAIN)), gcc)
	TOOLCHAIN := aarch64-none-elf
endif

ifeq ($(strip $(TOOLCHAIN)), clang)
	CC := clang -target aarch64-none-elf
	LD := ld.lld
	AR := llvm-ar
	RANLIB := llvm-ranlib
else
	CC := $(TOOLCHAIN)-gcc
	LD := $(TOOLCHAIN)-ld
	AS := $(TOOLCHAIN)-as
	AR := $(TOOLCHAIN)-ar
	RANLIB := $(TOOLCHAIN)-ranlib
endif

QEMU := qemu-system-aarch64

TOP := ${SDDF}/examples/gpu
PROJECT_INCLUDE := ${TOP}/include

MICROKIT_TOOL := $(MICROKIT_SDK)/bin/microkit

BOARD_DIR := $(MICROKIT_SDK)/board/$(MICROKIT_BOARD)/$(MICROKIT_CONFIG)

IMAGES := gpu_driver.elf client.elf gpu_virt.elf timer_driver.elf
CFLAGS := -mcpu=$(CPU) \
		  -mstrict-align \
		  -nostdlib \
		  -ffreestanding \
		  -g3 \
		  -O3 \
		  -Wall -Wno-unused-function -Werror -Wno-unused-command-line-argument \
		  -I$(BOARD_DIR)/include \
		  -I$(SDDF)/include \
		  -I$(PROJECT_INCLUDE)
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := --start-group -lmicrokit -Tmicrokit.ld libsddf_util_debug.a --end-group

IMAGE_FILE   := loader.img
REPORT_FILE  := report.txt
SYSTEM_FILE  := ${TOP}/board/$(MICROKIT_BOARD)/gpu.system

GPU_DRIVER   := $(SDDF)/drivers/gpu/${GPU_DRIVER_DIR}
GPU_COMPONENTS := $(SDDF)/gpu/components
TIMER_DRIVER := $(SDDF)/drivers/timer/$(TIMER_DRIVER_DIR)

all: $(IMAGE_FILE)

include ${SDDF}/util/util.mk
include ${GPU_DRIVER}/gpu_driver.mk
include ${GPU_COMPONENTS}/gpu_components.mk
include ${TIMER_DRIVER}/timer_driver.mk

${IMAGES}: libsddf_util_debug.a

fb_img.bgra: ${FB_IMG}
	convert -auto-orient -depth 8 -size $(shell convert $< -print "%wx%h" /dev/null) $< bgra:$@

client.o: ${TOP}/client.c ${FB_IMG}
	$(CC) -c $(CFLAGS) \
		-DFB_IMG_WIDTH=$(shell convert $(word $(words $^),$^) -auto-orient -print "%w" /dev/null) \
		-DFB_IMG_HEIGHT=$(shell convert $(word $(words $^),$^) -auto-orient -print "%h" /dev/null) \
		$< -o $@

fb_img.o: ${TOP}/fb_img.S fb_img.bgra
	$(CC) -c -DFB_IMG_PATH=\"$(word $(words $^),$^)\" $< -o $@

client.elf: client.o fb_img.o
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE) $(REPORT_FILE): $(IMAGES) $(SYSTEM_FILE)
	$(MICROKIT_TOOL) $(SYSTEM_FILE) --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)

qemu: $(IMAGE_FILE)
	sudo $(QEMU) -machine virt,virtualization=on,memory-backend=main-mem \
			-cpu $(CPU) \
			-serial mon:stdio \
			-device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0 \
			-m size=2G \
			-object memory-backend-memfd,id=main-mem,size=2G \
			-device virtio-gpu-device,edid=off,blob=on,max_outputs=1,indirect_desc=off,event_idx=off \
			-global virtio-mmio.force-legacy=false \
			-d guest_errors
			# -trace enable=virtio*

clean::
	rm -f client.o fb_img.o fb_img.bgra
clobber:: clean
	rm -f client.elf ${IMAGE_FILE} ${REPORT_FILE}
