/* Copyright 2026 Roos Catling-Tate
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

#ifndef __PIMCP2515_INTERNAL_H__
#define __PIMCP2515_INTERNAL_H__

#include <pi_MCP2515.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/*! @cond DOXYGEN_IGNORE */

#define CS_LOW(x) mcp2515_gpio_put(x, x->cs_pin, 0)
#define CS_HIGH(x) mcp2515_gpio_put(x, x->cs_pin, 1)

#ifdef NO_DEBUG
#define MCP2515_DEBUG(x, y, ...) (void)0/* NOOP */
#else
#define MCP2515_DEBUG(x, y, ...) __mcp2515_debug(x, y, ##__VA_ARGS__)
#endif

#ifdef USE_PICO_LIB
#include "hardware/spi.h"
#endif /* USE_PICO_LIB */

#define PI_MCP2515_GPIO_PIN_MAP_LEN 26

struct pi_mcp2515 {
	void (*callback)(char *, va_list);
	uint8_t cs_pin;
	uint32_t spi_clock;
	uint8_t osc_mhz;
	uint8_t spi_channel;
	uint8_t sck_pin;
	uint8_t tx_pin;
	uint8_t rx_pin;
#ifdef USE_PICO_LIB
	spi_inst_t *gpio_spi_inst;
#elif defined(USE_SPI)
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

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

int	mcp2515_gpio_init(pi_mcp2515_t *, uint8_t);
int	mcp2515_gpio_set_dir(const pi_mcp2515_t *, uint8_t gpio, bool out);
void	mcp2515_gpio_spi_free(const pi_mcp2515_t *);
int	mcp2515_gpio_spi_init(pi_mcp2515_t *);
int	mcp2515_gpio_spi_init_full_optional(pi_mcp2515_t *, uint8_t, uint8_t);
int	mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
int	mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
int	mcp2515_gpio_put(const pi_mcp2515_t *, uint8_t, uint8_t);

#ifndef NO_DEBUG
void	__mcp2515_debug(pi_mcp2515_t *, char *, ...);
#endif

/*! @endcond */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __PIMCP2515_INTERNAL_H__ */