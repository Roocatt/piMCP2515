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

#include "../include/pi_MCP2515_defs.h"
#include "pi_MCP2515_handle.h"
#include "registers.h"

#include "interrupt.h"

/**
 * @defgroup piMCP2515_interrupt_functions Interrupt Functions
 * @brief These functions handle interrupt related functionality.
 * @{
 */
/**
 * @brief Check for interrupts.
 *
 * The CANINTF register holds the interrupt flags.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the value of the interrupts flags.
 */
uint8_t
mcp2515_interrupts_get(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t intf;

	mcp2515_register_read(pi_mcp2515, &intf, 1, PI_MCP2515_RGSTR_CANINTF);

	return (intf);
}

/**
 * @brief Retrieve the interrupt mask.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the interrupt mask value.
 */
uint8_t
mcp2515_interrupts_mask(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t mask;

	mcp2515_register_read(pi_mcp2515, &mask, 1, PI_MCP2515_RGSTR_CANINTE);

	return (mask);
}

/**
 * @brief Clear all interrupts.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 */
void
mcp2515_interrupts_clear(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t zero = 0;

	mcp2515_register_write(pi_mcp2515, &zero, 1, PI_MCP2515_RGSTR_CANINTF);
}
/** @} */