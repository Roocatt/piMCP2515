/* Copyright 2025 Roos Catling-Tate
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

#include "pico/stdlib.h"

#include "pi_MCP2515.h"

void
mcp2515_message_send(pi_mcp2515_t *pi_mcp2515)
{

}

void
mcp2515_message_read(pi_mcp2515_t *pi_mcp2515)
{

}

void
mcp2515_read(pi_mcp2515_t *pi_mcp2515, uint8_t data[], uint8_t len, uint8_t rgstr)
{
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = MCP2515_INSTR_READ;
	message[1] = rgstr;
	spi_write_blocking(pi_mcp2515->channel, message, 2);
	spi_read_blocking(pi_mcp2515->channel, 0x00, data, len);
	UNSET_CS(pi_mcp2515);
}

void
mcp2515_write(pi_mcp2515_t *pi_mcp2515, uint8_t values[], uint8_t len, uint8_t rgstr)
{
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = MCP2515_INSTR_WRITE;
	message[1] = rgstr;
	spi_write_blocking(pi_mcp2515->channel, message, 2);
	spi_write_blocking(pi_mcp2515->channel, values, len);
	UNSET_CS(pi_mcp2515);
}

int
mcp2515_mode(pi_mcp2515_t *pi_mcp2515)
{
	return (0); /* TODO */
}

int
mcp2515_init(pi_mcp2515_t *pi_mcp2515, spi_inst_t *channel, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin,
		uint8_t sck_pin, uint32_t spi_clock)
{
	pi_mcp2515->channel = channel;
	pi_mcp2515->cs_pin = cs_pin;
	pi_mcp2515->sck_pin = sck_pin;
	pi_mcp2515->tx_pin = tx_pin;
	pi_mcp2515->rx_pin = rx_pin;
	pi_mcp2515->spi_clock = spi_clock;

	spi_init(channel, spi_clock);
	gpio_set_function(tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(rx_pin, GPIO_FUNC_SPI);
	gpio_set_function(sck_pin, GPIO_FUNC_SPI);

	gpio_init(cs_pin);
	gpio_set_dir(cs_pin, GPIO_OUT);

	UNSET_CS(pi_mcp2515);

	return (0);
}
