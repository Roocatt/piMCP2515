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

#include <stdint.h>

#include "../include/pi_MCP2515_defs.h"

#include "registers.h"
#include "pi_MCP2515.h"
#include "gpio.h"
#include "time.h"

#include "reqop.h"

/**
 * @defgroup piMCP2515_reqop_functions REQOP (Operating Mode) Functions
 * @brief These functions handle REQOP (operating mode) functionality.
 * @{
 */
/**
 * @brief Issue a reset command to the MCP2515 via the SPI interface.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_reset(pi_mcp2515_t *pi_mcp2515)
{
	int res;
	uint8_t instr = PI_MCP2515_INSTR_RESET, blank[14] = { 0 };

	SET_CS(pi_mcp2515);
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
	UNSET_CS(pi_mcp2515);

	if (res)
		goto err;

	mcp2515_micro_sleep(mcp2515_osc_time(pi_mcp2515, MCP2515_REQOP_CHANGE_SLEEP_CYCLES));

	if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), 0x30)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), 0x40)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), 0x50)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX0CTRL)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX1CTRL);

	err:
		return (res);
}

/**
 * @brief Change the operating mode.
 *
 * After the mode change, this code will then sleep for a 128 oscillator cycles delay before continuing operations as
 * defined by the MCP2515 manual.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param reqop the new operating mode.
 *              Acceptable values being:
 *              - PI_MCP2515_REQOP_MASK
 *              - PI_MCP2515_REQOP_NORMAL
 *              - PI_MCP2515_REQOP_SLEEP
 *              - PI_MCP2515_REQOP_LOOPBACK
 *              - PI_MCP2515_REQOP_LISTENONLY
 *              - PI_MCP2515_REQOP_CONFIG
 *              - PI_MCP2515_REQOP_POWERUP
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_reqop(pi_mcp2515_t *pi_mcp2515, uint8_t reqop)
{
	int res;

	res = mcp2515_register_bitmod(pi_mcp2515, reqop, PI_MCP2515_REQOP_MASK, PI_MCP2515_RGSTR_CANCTRL);
	mcp2515_micro_sleep(mcp2515_osc_time(pi_mcp2515, MCP2515_REQOP_CHANGE_SLEEP_CYCLES));

	return (res);
}

/**
 * @brief Get the current operating mode.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return the current operating mode.
 */
uint8_t
mcp2515_reqop_get(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t reqop;

	mcp2515_register_read(pi_mcp2515, &reqop, 1, PI_MCP2515_RGSTR_CANSTAT);

	return (reqop & PI_MCP2515_REQOP_MASK);
}
/** @} */