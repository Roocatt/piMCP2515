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

#include "gpio.h"
#include "pi_MCP2515_defs.h"
#include "pi_MCP2515_handle.h"

#include "registers.h"

/**
 * @defgroup piMCP2515_register_functions Register Functions
 * @brief These functions handle manipulating values in the MCP2515's registers.
 * @{
 */
/**
 * @brief Read a value from a specified register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param data the destination to copy the read data to.
 * @param len the length of the destination buffer.
 * @param rgstr the register to read.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_register_read(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len, uint8_t rgstr)
{
	int res;
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_READ;
	message[1] = rgstr;
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, message, 2);
	if (res)
		goto err;
	res = mcp2515_gpio_spi_read_blocking(pi_mcp2515, data, len);

err:
	UNSET_CS(pi_mcp2515);

	return (res);
}

/**
 * @brief Write a value to the specified register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param values the data to write to the register.
 * @param len the length of the data to write.
 * @param rgstr the register to write to.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_register_write(pi_mcp2515_t *pi_mcp2515, uint8_t values[], uint8_t len, uint8_t rgstr)
{
	int res;
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_WRITE;
	message[1] = rgstr;

	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, message, 2);
	if (res)
		goto err;
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, values, len);

err:
	UNSET_CS(pi_mcp2515);

	return (res);
}

/**
 * @brief Change the value of a register via an SPI bit modify.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param data the data to use for the bitmod.
 * @param mask the mask to apply where only bits with a 1 value will be updated in the register with the data values.
 * @param rgstr the register to perform the bitmod against.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_register_bitmod(pi_mcp2515_t *pi_mcp2515, uint8_t data, uint8_t mask, uint8_t rgstr)
{
	int res;
	uint8_t message[4];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_BITMOD;
	message[1] = rgstr;
	message[2] = mask;
	message[3] = data;
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, message, 4);
	UNSET_CS(pi_mcp2515);

	return (res);
}
/** @} */