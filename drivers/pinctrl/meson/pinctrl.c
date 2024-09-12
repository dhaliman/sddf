/*
 * Copyright 2024, UNSW
 * SPDX-License-Identifier: BSD-2-Clause
 */

// Pinctrl driver. Tested on OdroidC4 (Amlogic S903X3)

// Documents referenced:
// Linux: drivers/pinctrl/meson/pinctrl-meson-g12a.c
// Linux: drivers/pinctrl/meson/pinctrl-meson.c

#include <microkit.h>
#include <stdint.h>
#include <stdbool.h>
#include <sddf/util/printf.h>
#include <sddf/pinctrl/protocol.h>

// Logging
#define DEBUG_DRIVER

#ifdef DEBUG_DRIVER
#define LOG_DRIVER(...) do{ sddf_dprintf("PINCTRL DRIVER|INFO: "); sddf_dprintf(__VA_ARGS__); }while(0)
#else
#define LOG_DRIVER(...) do{}while(0)
#endif

#define LOG_DRIVER_ERR(...) do{ sddf_printf("PINCTRL DRIVER|ERROR: "); sddf_printf(__VA_ARGS__); }while(0)

// Memory definitions
#define MUX_REG_ADDR(base, offset) ((uint32_t *) (base + offset))

typedef struct {
    uint32_t offset;
    uint32_t value;
} pindata_t;

#define PINMUX_DATA_MAGIC 0x73ABC62F

// Device mapped
uintptr_t pinctrl_ao_base;
uintptr_t pinctrl_periphs_base;

// Data from DTS prepared by Python script
extern pindata_t ao_registers[];
extern const uint32_t num_ao_registers;

extern pindata_t peripheral_registers[];
extern const uint32_t num_peripheral_registers;

extern const uint32_t pinmux_data_magic;

bool check_vaddr_4_bytes_aligned(uint32_t *vaddr) {
    if (((uintptr_t) vaddr) % 4 == 0) {
        return true;
    } else {
        LOG_DRIVER_ERR("vaddr is not 4 bytes aligned\n");
        return false;
    }
}

bool read_mux(uint32_t *vaddr, uint32_t *ret) {
    if (!check_vaddr_4_bytes_aligned(vaddr)) {
        return false;
    }

    asm volatile("" : : : "memory");
    *ret = *vaddr;
    asm volatile("" : : : "memory");

    return true;
}

bool set_mux(uint32_t *vaddr, uint32_t val) {
    if (!check_vaddr_4_bytes_aligned(vaddr)) {
        return false;
    }

    asm volatile("" : : : "memory");
    *vaddr = val;
    asm volatile("" : : : "memory");

    return true;
}

void init(void) {
    LOG_DRIVER("starting\n");

    if (pinmux_data_magic != PINMUX_DATA_MAGIC) {
        LOG_DRIVER_ERR("magic does not match");
    }

    // The peripherals pinmux device physical address isn't aligned on page boundary whereas
    // memory regions can only be mapped on page boundary, we need to offset the page boundary address
    // into the pinmux device.
    pinctrl_periphs_base += 0x400;

    for (uint32_t i = 0; i < num_peripheral_registers; i += 1) {
        set_mux(MUX_REG_ADDR(pinctrl_periphs_base, peripheral_registers[i].offset), peripheral_registers[i].value);
    }

    LOG_DRIVER("pinctrl device initialisation done\n");
}

void notified(microkit_channel ch) {
    LOG_DRIVER_ERR("received ntfn on unexpected channel %u\n", ch);
}

