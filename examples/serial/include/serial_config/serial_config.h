/*
 * Copyright 2024, UNSW
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <microkit.h>
#include <sddf/util/string.h>
#include <sddf/serial/queue.h>
#include <sddf/serial/util.h>
#include <stdint.h>

#define SERIAL_NUM_CLIENTS 2

/* Only support transmission and not receive. */
#define SERIAL_TX_ONLY 0

/* Associate a colour with each client's output. */
#define SERIAL_WITH_COLOUR 1

/* Control character to switch input stream - ctrl \. To input character input twice. */
#define SERIAL_SWITCH_CHAR 28

/* Control character to terminate client number input. */
#define SERIAL_TERMINATE_NUM '\r'

/* Default baud rate of the uart device */
#define UART_DEFAULT_BAUD 115200

/* String to be printed to start console input */
#define SERIAL_CONSOLE_BEGIN_STRING "Begin input\n"
#define SERIAL_CONSOLE_BEGIN_STRING_LEN 12

#define SERIAL_CLI0_NAME "client0"
#define SERIAL_CLI1_NAME "client1"
#define SERIAL_VIRT_RX_NAME "serial_virt_rx"
#define SERIAL_VIRT_TX_NAME "serial_virt_tx"

#define SERIAL_QUEUE_SIZE                          0x1000
#define SERIAL_DATA_REGION_SIZE                    0x2000

#define SERIAL_TX_DATA_REGION_SIZE_DRIV            (2 * SERIAL_DATA_REGION_SIZE)
#define SERIAL_TX_DATA_REGION_SIZE_CLI0            SERIAL_DATA_REGION_SIZE
#define SERIAL_TX_DATA_REGION_SIZE_CLI1            SERIAL_DATA_REGION_SIZE

#define SERIAL_RX_DATA_REGION_SIZE_DRIV            SERIAL_DATA_REGION_SIZE
#define SERIAL_RX_DATA_REGION_SIZE_CLI0            SERIAL_DATA_REGION_SIZE
#define SERIAL_RX_DATA_REGION_SIZE_CLI1            SERIAL_DATA_REGION_SIZE

#define SERIAL_MAX_CLIENT_TX_DATA_SIZE MAX(SERIAL_TX_DATA_REGION_SIZE_CLI0, SERIAL_TX_DATA_REGION_SIZE_CLI1)
#if SERIAL_WITH_COLOUR
_Static_assert(SERIAL_TX_DATA_REGION_SIZE_DRIV > SERIAL_MAX_CLIENT_TX_DATA_SIZE,
               "Driver TX data region must be larger than all client data regions in SERIAL_WITH_COLOUR mode.");
#endif

#define SERIAL_MAX_TX_DATA_SIZE MAX(SERIAL_TX_DATA_REGION_SIZE_DRIV, SERIAL_MAX_CLIENT_TX_DATA_SIZE)
#define SERIAL_MAX_RX_DATA_SIZE MAX(SERIAL_RX_DATA_REGION_SIZE_DRIV, MAX(SERIAL_RX_DATA_REGION_SIZE_CLI0, SERIAL_RX_DATA_REGION_SIZE_CLI1))
#define SERIAL_MAX_DATA_SIZE MAX(SERIAL_MAX_TX_DATA_SIZE, SERIAL_MAX_RX_DATA_SIZE)
_Static_assert(SERIAL_MAX_DATA_SIZE < UINT32_MAX,
               "Data regions must be smaller than UINT32 max to correctly use queue data structure.");

static inline void serial_cli_data_size(char *pd_name, size_t *rx_data_size, size_t* tx_data_size)
{
    if (!sddf_strcmp(pd_name, SERIAL_CLI0_NAME)) {
        *rx_data_size = SERIAL_RX_DATA_REGION_SIZE_CLI0;
        *tx_data_size = SERIAL_TX_DATA_REGION_SIZE_CLI0;
    } else if (!sddf_strcmp(pd_name, SERIAL_CLI1_NAME)) {
        *rx_data_size = SERIAL_RX_DATA_REGION_SIZE_CLI1;
        *tx_data_size = SERIAL_TX_DATA_REGION_SIZE_CLI1;
    }
}



static inline void serial_virt_queue_info(char *pd_name, serial_queue_t *cli0_queue, char *cli0_data,
                                          serial_info_t ret[SERIAL_NUM_CLIENTS])
{
    if (!sddf_strcmp(pd_name, SERIAL_VIRT_RX_NAME)) {
        ret[0] = (serial_info_t) {.queue = cli0_queue, .data = cli0_data, .data_size = SERIAL_RX_DATA_REGION_SIZE_CLI0};
        ret[1] = (serial_info_t) {.queue = (serial_queue_t *)((uintptr_t)cli0_queue + SERIAL_QUEUE_SIZE),
                                        .data = cli0_data + SERIAL_RX_DATA_REGION_SIZE_CLI0,
                                        .data_size = SERIAL_RX_DATA_REGION_SIZE_CLI1};
    } else if (!sddf_strcmp(pd_name, SERIAL_VIRT_TX_NAME)) {
        ret[0] = (serial_info_t) {.queue = cli0_queue, .data = cli0_data, .data_size = SERIAL_TX_DATA_REGION_SIZE_CLI0};
        ret[1] = (serial_info_t) {.queue = (serial_queue_t *)((uintptr_t)cli0_queue + SERIAL_QUEUE_SIZE),
                                .data = cli0_data + SERIAL_TX_DATA_REGION_SIZE_CLI0,
                                .data_size = SERIAL_TX_DATA_REGION_SIZE_CLI1};
    }
}

#if SERIAL_WITH_COLOUR
static inline void serial_channel_names_init(char **client_names)
{
    client_names[0] = SERIAL_CLI0_NAME;
    client_names[1] = SERIAL_CLI1_NAME;
}
#endif