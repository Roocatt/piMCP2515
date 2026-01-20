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

/* External Header.
 *
 * This is for including in projects that use the library.
 *
 * TODO First draft. Tidy and finish the external header stuff.
 */

#ifndef PIMCP2515_PI_MCP2515_H
#define PIMCP2515_PI_MCP2515_H

#include <stdbool.h>
#include <stdint.h>

typedef struct pi_mcp2515 pi_mcp2515_t;
typedef struct pi_mcp2515_can_frame pi_mcp2515_can_frame_t;

int		mcp2515_can_message_send(pi_mcp2515_t *, const pi_mcp2515_can_frame_t *);
int		mcp2515_can_message_read(pi_mcp2515_t *, pi_mcp2515_can_frame_t *);
bool		mcp2515_can_message_received(pi_mcp2515_t *);

uint8_t		mcp2515_interrupts_get(pi_mcp2515_t *);
uint8_t		mcp2515_interrupts_mask(pi_mcp2515_t *);
void		mcp2515_interrupts_clear(pi_mcp2515_t *);

int		mcp2515_filter(pi_mcp2515_t *, uint8_t, uint32_t, bool);
int		mcp2515_filter_mask(pi_mcp2515_t *, uint8_t, int32_t, bool);

int		mcp2515_register_read(pi_mcp2515_t *, uint8_t *, uint8_t, uint8_t);
int		mcp2515_register_write(pi_mcp2515_t *, uint8_t[], uint8_t, uint8_t);
int		mcp2515_register_bitmod(pi_mcp2515_t *, uint8_t, uint8_t, uint8_t);

int		mcp2515_reqop(pi_mcp2515_t *, uint8_t);
uint8_t		mcp2515_reqop_get(pi_mcp2515_t *);
uint8_t		mcp2515_status(pi_mcp2515_t *);
uint8_t		mcp2515_error_tx_count(pi_mcp2515_t *);
uint8_t		mcp2515_error_rx_count(pi_mcp2515_t *);
uint8_t		mcp2515_error_flags(pi_mcp2515_t *);
bool		mcp2515_error(pi_mcp2515_t *);
int		mcp2515_error_clear_errif(pi_mcp2515_t *);

int		mcp2515_bitrate_default_16mhz_1000kbps(pi_mcp2515_t *);
int		mcp2515_bitrate_default_8mhz_500kbps(pi_mcp2515_t *);
int		mcp2515_bitrate_simplified(pi_mcp2515_t *, uint16_t);
int		mcp2515_bitrate_full_optional(pi_mcp2515_t *, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    bool, bool, bool, bool);
int		mcp2515_reset(pi_mcp2515_t *);

uint64_t	mcp2515_osc_time(const pi_mcp2515_t *, uint32_t);
void		mcp2515_free(pi_mcp2515_t *);
int		mcp2515_init(pi_mcp2515_t **, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t);
void		mcp2515_conf_spi_devpath(pi_mcp2515_t *, char *);
void		mcp2515_conf_gpio_devpath(pi_mcp2515_t *, char *);

#endif /* PIMCP2515_PI_MCP2515_H */