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

#include <stdbool.h>

#include "pi_MCP2515_handle.h"
#include "../include/pi_MCP2515_defs.h"
#include "gpio.h"
#include "registers.h"

#include "status_error.h"

#include "debug.h"

/**
 * @defgroup piMCP2515_error_status_functions Error/Status Functions
 * @brief These functions handle Status and Errors.
 * @{
 */
/**
 * @brief Check status via the STATUS instruction.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the current status value.
 */
uint8_t
mcp2515_status(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t instruction, res;

	CS_LOW(pi_mcp2515);
	instruction = PI_MCP2515_INSTR_READ_STATUS;
	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instruction, 1);
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, &res, 1);
	CS_HIGH(pi_mcp2515);

	MCP2515_DEBUG(pi_mcp2515, "MCP2515 status 0x%04x\n", res);

	return (res);
}

/**
 * @brief Check the value of the 'transmit' (TX) error counter.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the TX error count.
 */
uint8_t
mcp2515_error_tx_count(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t res = 0;

	mcp2515_register_read(pi_mcp2515, &res, 1, PI_MCP2515_RGSTR_ECTX);

	return (res);
}

/**
 * @brief Check the value of the 'receive' (RX) error counter.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the RX error count.
 */
uint8_t
mcp2515_error_rx_count(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t res = 0;

	mcp2515_register_read(pi_mcp2515, &res, 1, PI_MCP2515_RGSTR_ECRX);

	return (res);
}

/**
 * @brief Fetch the contents of the error flag (EFLG) register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return contents of the error flag (EFLG) register.
 */
uint8_t
mcp2515_error_flags(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t flags = 0;

	mcp2515_register_read(pi_mcp2515, &flags, 1, PI_MCP2515_RGSTR_EFLG);

	return (flags);
}

/**
 * @brief Check error flags in the EFLG register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return true if any error flags are set.
 */
bool
mcp2515_error(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_error_flags(pi_mcp2515) & PI_MCP2515_EFLG_MASK);
}

/**
 * @brief Clear error interrupt flag.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_error_clear_errif(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_register_bitmod(pi_mcp2515, 0, PI_MCP2515_CANINTF_ERRIF, PI_MCP2515_RGSTR_CANINTF));
}
/** @} */
