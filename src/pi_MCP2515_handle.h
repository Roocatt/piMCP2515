/* Copyright 2025-2026 Roos Catling-Tate
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or
 * without fee is hereby granted, provided that the above copyright notice and this permission
 * notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PIMCP2515_PI_MCP2515_HANDLE_H
#define PIMCP2515_PI_MCP2515_HANDLE_H

#include <stdint.h>

#ifdef USE_PICO_LIB
#include "hardware/spi.h"
#endif /* USE_PICO_LIB */

/* Library Definitions */
#define PI_MCP2515_GPIO_PIN_MAP_LEN 26

struct pi_mcp2515 {
	uint8_t cs_pin;
	uint32_t spi_clock;
	uint8_t osc_mhz;
	uint8_t spi_channel;
	uint8_t sck_pin;
	uint8_t tx_pin;
	uint8_t rx_pin;
#ifdef USE_PICO_LIB
	spi_inst_t *gpio_spi_inst;
#elif defined(USE_SPIDEV)
	char *gpio_dev_spi_path;
	char *gpio_dev_gpio_path;
	int gpio_spidev_fd;
	int gpio_gpio_fd;
	uint8_t gpio_spi_mode;
#ifdef __linux__
	uint8_t gpio_spi_bits_per_word;
	uint16_t gpio_spi_delay_usec;
	int gpio_pin_fd_map[PI_MCP2515_GPIO_PIN_MAP_LEN];
#endif /* __linux__ */
#endif /* USE_PICO_LIB */
};

typedef struct pi_mcp2515 pi_mcp2515_t;

#endif /* PIMCP2515_PI_MCP2515_HANDLE_H */