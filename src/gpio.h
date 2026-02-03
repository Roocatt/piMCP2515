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

/* This file is for conditionally selecting which backend SPI/GPIO functionality is used. For instance, the Pi Pico SDK
 * or using print debugging so that logic can be tested locally.
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

#include "pi_MCP2515_handle.h"

#define SET_CS(x) mcp2515_gpio_put(x, x->cs_pin, 0)
#define UNSET_CS(x) mcp2515_gpio_put(x, x->cs_pin, 1)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int	mcp2515_gpio_init(pi_mcp2515_t *, uint8_t);
int	mcp2515_gpio_set_dir(const pi_mcp2515_t *, uint8_t gpio, bool out);
void mcp2515_gpio_spi_free(const pi_mcp2515_t *);
int	mcp2515_gpio_spi_init(pi_mcp2515_t *);
int	mcp2515_gpio_spi_init_full_optional(pi_mcp2515_t *, uint8_t, uint8_t);
int	mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
int	mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
int	mcp2515_gpio_put(const pi_mcp2515_t *, uint8_t, uint8_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GPIO_H */
