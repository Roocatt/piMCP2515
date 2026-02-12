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

#ifndef PIMCP2515_CAN_H
#define PIMCP2515_CAN_H

#include <stdbool.h>
#include <stdint.h>

#include "pi_MCP2515_handle.h"

/**
 * @brief CAN bus frame data structure.
 */
typedef struct {
	uint32_t id;
	uint8_t dlc;
	uint8_t payload[8];
} pi_mcp2515_can_frame_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint32_t	mcp2515_can_id_build(uint32_t, bool);

int	mcp2515_can_clear_txif(pi_mcp2515_t *, uint8_t);
int	mcp2515_can_message_send(pi_mcp2515_t *, const pi_mcp2515_can_frame_t *);
int	mcp2515_can_message_read(pi_mcp2515_t *, pi_mcp2515_can_frame_t *);
bool	mcp2515_can_message_received(pi_mcp2515_t *);

void	mcp2515_rts(pi_mcp2515_t *, uint8_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PIMCP2515_CAN_H */