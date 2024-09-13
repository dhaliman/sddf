#
# Copyright 2024, UNSW
#
# SPDX-License-Identifier: BSD-2-Clause
#

ifeq ($(strip $(MICROKIT_SDK)),)
$(error MICROKIT_SDK must be specified)
endif

ifeq ($(strip $(MICROKIT_BOARD)),)
$(error MICROKIT_BOARD must be specified)
endif

FB_IMG ?= fb_img.jpeg

ifeq ($(wildcard $(FB_IMG)),)
$(error FB_IMG file ${FB_IMG} does not exist)
endif

BUILD_DIR ?= build

override BUILD_DIR := $(abspath ${BUILD_DIR})
override FB_IMG := $(abspath ${FB_IMG})
override MICROKIT_SDK := $(abspath ${MICROKIT_SDK})

export MICROKIT_BOARD
export MICROKIT_CONFIG ?= debug
export SDDF := $(abspath ../..)

IMAGE_FILE:= ${BUILD_DIR}/loader.img
REPORT_FILE:= ${BUILD_DIR}/report.txt

all: ${IMAGE_FILE}

qemu ${IMAGE_FILE} ${REPORT_FILE} clean clobber: ${BUILD_DIR}/Makefile FORCE
	${MAKE} -C ${BUILD_DIR} MICROKIT_SDK=${MICROKIT_SDK} BUILD_DIR=${BUILD_DIR} FB_IMG=${FB_IMG} $(notdir $@)

${BUILD_DIR}/Makefile: gpu.mk
	mkdir -p ${BUILD_DIR}
	cp gpu.mk $@

FORCE:
