
#ifndef FLASH_H
#define FLASH_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/flash.h"

#define FLASH_TARGET_OFFSET (256 * 1024)

void print_buf(const uint8_t *buf, size_t len);

extern uint8_t *flashData;

#endif