#
# Copyright 2024, UNSW
#
# SPDX-License-Identifier: BSD-2-Clause
#
# This Makefile snippet builds the gpu virtualiser
# it should be included into your project Makefile
#
# NOTES:
#  Generates gpu_virt.elf
#


GPU_IMAGES := gpu_virt.elf

CFLAGS_gpu ?=

CHECK_GPU_FLAGS_MD5:=.gpu_cflags-$(shell echo -- ${CFLAGS} ${CFLAGS_gpu} | shasum | sed 's/ *-//')

${CHECK_GPU_FLAGS_MD5}:
	-rm -f .gpu_cflags-*
	touch $@


gpu_virt.elf: gpu_virt.o
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

gpu_virt.o: ${CHECK_GPU_FLAGS_MD5}
gpu_virt.o: ${SDDF}/gpu/components/virt.c
	${CC} ${CFLAGS} ${CFLAGS_gpu} -o $@ -c $<

clean::
	rm -f gpu_virt.[od] .gpu_cflags-*

clobber::
	rm -f ${GPU_IMAGES}


-include gpu_virt.d
