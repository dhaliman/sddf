#
# Copyright 2022, UNSW
#
# SPDX-License-Identifier: BSD-2-Clause
#

ifeq ($(strip $(SDDF_LWIP_NUM_BUFS)),)
$(error SDDF_LWIP_NUM_BUFS must be specified)
endif

LIB_SDDF_LWIP_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIB_SDDF_LWIP_FILES := $(addprefix ${LIB_SDDF_LWIP_DIR}/, lib_sddf_lwip.c)
LIB_SDDF_LWIP_OBJS := $(LIB_SDDF_LWIP_FILES:.c=.o)

${LIB_SDDF_LWIP_OBJS}: CFLAGS += -DSDDF_LWIP_NUM_BUFS=$(SDDF_LWIP_NUM_BUFS)

${LIB_SDDF_LWIP_OBJS}: ${CHECK_FLAGS_BOARD_MD5} | ${LIB_SDDF_LWIP_DIR}
${LIB_SDDF_LWIP_DIR}:
	mkdir -p $@

lib_sddf_lwip.a: ${LIB_SDDF_LWIP_OBJS}
	${AR} rv $@ $^
	${RANLIB} $@

clean::
	${RM} -f ${LIB_SDDF_LWIP_OBJS} ${LIB_SDDF_LWIP_OBJS:.o=.d}

clobber:: clean
	${RM} -f lib_sddf_lwip.a
	rmdir ${LIB_SDDF_LWIP_DIR}

-include ${LIB_SDDF_LWIP_OBJS:.o=.d}
