/*
 * Copyright 2024, UNSW
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <microkit.h>
#include <sddf/util/string.h>
#include <sddf/serial/queue.h>
#include <stdint.h>

/* Number of clients that can be connected to the serial server. */
#define NUM_SERIAL_CLIENTS 3

/* Only support transmission and not receive. */
#define SERIAL_TX_ONLY 1

/* Associate a colour with each client's output. */
#define SERIAL_WITH_COLOUR 1

/* Control character to switch input stream - ctrl \. To input character input twice. */
#define SERIAL_SWITCH_CHAR 28

/* Control character to terminate client number input. */
#define SERIAL_TERMINATE_NUM '\r'

/* Default baud rate of the uart device */
#define UART_DEFAULT_BAUD 115200

#define SERIAL_CLI0_NAME "client0"
#define SERIAL_CLI1_NAME "client1"
#define SERIAL_CLI2_NAME "bench0"
#define SERIAL_VIRT_TX_NAME "serial_virt_tx"

#define SERIAL_QUEUE_SIZE                          0x1000
#define SERIAL_DATA_REGION_SIZE                    0x2000

#define SERIAL_TX_DATA_REGION_SIZE_DRIV            (2 * SERIAL_DATA_REGION_SIZE)
#define SERIAL_TX_DATA_REGION_SIZE_CLI0            SERIAL_DATA_REGION_SIZE
#define SERIAL_TX_DATA_REGION_SIZE_CLI1            SERIAL_DATA_REGION_SIZE
#define SERIAL_TX_DATA_REGION_SIZE_CLI2            SERIAL_DATA_REGION_SIZE

/* To avoid deadlocks caused when the virtualiser adds colour codes to the
   start and end of strings, driver data region must be larger than any
   client data region. */
#define SERIAL_MAX_CLIENT_TX_DATA_SIZE MAX(SERIAL_TX_DATA_REGION_SIZE_CLI2, MAX(SERIAL_TX_DATA_REGION_SIZE_CLI0, SERIAL_TX_DATA_REGION_SIZE_CLI1))
#if SERIAL_WITH_COLOUR
_Static_assert(SERIAL_TX_DATA_REGION_SIZE_DRIV > SERIAL_MAX_CLIENT_TX_DATA_SIZE,
               "Driver TX data region must be larger than all client data regions in SERIAL_WITH_COLOUR mode.");
#endif

/* Ensure the entire data region can be assigned a unique index by a 32 bit
   unsigned. */
#define SERIAL_MAX_DATA_SIZE MAX(SERIAL_TX_DATA_REGION_SIZE_DRIV, SERIAL_MAX_CLIENT_TX_DATA_SIZE)
_Static_assert(SERIAL_MAX_DATA_SIZE < UINT32_MAX,
               "Data regions must be smaller than UINT32 max to correctly use queue data structure.");

static inline void serial_cli_data_size(char *pd_name, uint32_t *rx_data_size, uint32_t *tx_data_size)
{
    if (!sddf_strcmp(pd_name, SERIAL_CLI0_NAME)) {
        *tx_data_size = SERIAL_TX_DATA_REGION_SIZE_CLI0;
    } else if (!sddf_strcmp(pd_name, SERIAL_CLI1_NAME)) {
        *tx_data_size = SERIAL_TX_DATA_REGION_SIZE_CLI1;
    } else if (!sddf_strcmp(pd_name, SERIAL_CLI2_NAME)) {
        *tx_data_size = SERIAL_TX_DATA_REGION_SIZE_CLI2;
    }
}

typedef struct serial_queue_info {
    serial_queue_t *cli_queue;
    char *cli_data;
    uint32_t size;
} serial_queue_info_t;

static inline void serial_virt_queue_init_sys(char *pd_name, serial_queue_t *cli_queue,
                                              char *cli_data, serial_queue_info_t ret[NUM_SERIAL_CLIENTS])
{
    if (!sddf_strcmp(pd_name, SERIAL_VIRT_TX_NAME)) {
        ret[0] = (serial_queue_info_t) {.cli_queue = cli_queue, .cli_data = cli_data, .size = SERIAL_TX_DATA_REGION_SIZE_CLI0};
        ret[1] = (serial_queue_info_t) {.cli_queue = (serial_queue_t *)((uintptr_t)ret[0].cli_queue + SERIAL_QUEUE_SIZE),
                                      .cli_data = ret[0].cli_data + ret[0].size, .size = SERIAL_TX_DATA_REGION_SIZE_CLI1};
        ret[2] = (serial_queue_info_t) {.cli_queue = (serial_queue_t *)((uintptr_t)ret[1].cli_queue + SERIAL_QUEUE_SIZE),
                                      .cli_data = ret[1].cli_data + ret[1].size, .size = SERIAL_TX_DATA_REGION_SIZE_CLI2};
    }
}

#if SERIAL_WITH_COLOUR
static inline void serial_channel_names_init(char *pd_name, char *client_names[NUM_SERIAL_CLIENTS])
{
    if (!sddf_strcmp(pd_name, SERIAL_VIRT_TX_NAME)) {
        client_names[0] = SERIAL_CLI0_NAME;
        client_names[1] = SERIAL_CLI1_NAME;
        client_names[2] = SERIAL_CLI2_NAME;
    }
}
#endif
