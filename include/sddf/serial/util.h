#pragma once

#include <sddf/serial/queue.h>

typedef struct serial_info {
    serial_queue_t *queue;
    char *data;
    size_t data_size;
} serial_info_t;