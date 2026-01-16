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

#ifndef PI_MCP2515_H
#define PI_MCP2515_H

#include <stdbool.h>
#include <stdint.h>

#include "pi_MCP2515_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int	mcp2515_bitrate_default_16mhz_1000kbps(pi_mcp2515_t *);
int	mcp2515_bitrate_default_8mhz_500kbps(pi_mcp2515_t *);
int	mcp2515_bitrate_simplified(pi_mcp2515_t *, uint16_t);
int	mcp2515_bitrate_full_optional(pi_mcp2515_t *, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, bool, bool,
    bool, bool);
void	mcp2515_free(pi_mcp2515_t *);
int	mcp2515_init(pi_mcp2515_t **, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t);

void	mcp2515_conf_spi_devpath(pi_mcp2515_t *, char *);
void	mcp2515_conf_gpio_devpath(pi_mcp2515_t *, char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PI_MCP2515_H */
