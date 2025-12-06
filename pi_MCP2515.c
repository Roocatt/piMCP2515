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
mcp2515_can_message_send(pi_mcp2515_t *pi_mcp2515, pi_mcp2515_can_frame_t *can_frame)
{

}

void
mcp2515_can_message_read(pi_mcp2515_t *pi_mcp2515, pi_mcp2515_can_frame_t *can_frame)
{
	uint32_t id;
	uint8_t buffer[5], status, dlc, ctrl, ctrl_reg, sidh_reg, data_reg, canintf;

	SET_CS(pi_mcp2515);
	status = mcp2515_status(pi_mcp2515);
	if (status & PI_MCP2515_STATUS_RX0BF) {
		ctrl_reg = 0x60;
		sidh_reg = 0x61;
		data_reg = 0x66;
		canintf = 0x01;
	} else if (status & PI_MCP2515_STATUS_RX1BF) {
		ctrl_reg = 0x70;
		sidh_reg = 0x71;
		data_reg = 0x76;
		canintf = 0x02;
	} else {
		/* TODO Error handle */
		return;
	}
	mcp2515_register_read(pi_mcp2515, buffer, 5, sidh_reg);

	id = (buffer[0] << 3) | (buffer[1] >> 5); /* TODO Missing expanded id */
	dlc = buffer[4] & 0x0F;

	mcp2515_register_read(pi_mcp2515, &ctrl, 1, ctrl_reg);

	/* TODO incomplete */
	can_frame->id = id;
	can_frame->dlc = dlc;

	mcp2515_register_read(pi_mcp2515, buffer, dlc, data_reg);
	mcp2515_register_bitmod(pi_mcp2515, 0, canintf, PI_MCP2515_RGSTR_CANINTF);

	UNSET_CS(pi_mcp2515);
}

uint8_t
mcp2515_status(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t instruction, res;

	SET_CS(pi_mcp2515);
	instruction = PI_MCP2515_INSTR_READ_STATUS;
	spi_write_blocking(pi_mcp2515->spi_channel, &instruction, 1);
	spi_read_blocking(pi_mcp2515->spi_channel, 0x00, &res, 1);
	UNSET_CS(pi_mcp2515);

	return (res);
}

void
mcp2515_register_read(pi_mcp2515_t *pi_mcp2515, uint8_t data[], uint8_t len, uint8_t rgstr)
{
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_READ;
	message[1] = rgstr;
	spi_write_blocking(pi_mcp2515->spi_channel, message, 2);
	spi_read_blocking(pi_mcp2515->spi_channel, 0x00, data, len);
	UNSET_CS(pi_mcp2515);
}

void
mcp2515_register_write(pi_mcp2515_t *pi_mcp2515, uint8_t values[], uint8_t len, uint8_t rgstr)
{
	uint8_t message[2];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_WRITE;
	message[1] = rgstr;
	spi_write_blocking(pi_mcp2515->spi_channel, message, 2);
	spi_write_blocking(pi_mcp2515->spi_channel, values, len);
	UNSET_CS(pi_mcp2515);
}

void
mcp2515_register_bitmod(pi_mcp2515_t *pi_mcp2515, uint8_t data, uint8_t mask, uint8_t rgstr)
{
	uint8_t message[4];

	SET_CS(pi_mcp2515);
	message[0] = PI_MCP2515_INSTR_BITMOD;
	message[1] = rgstr;
	message[2] = mask;
	message[3] = data;
	spi_write_blocking(pi_mcp2515->spi_channel, message, 4);
	UNSET_CS(pi_mcp2515);
}

void
mcp2515_reqop(pi_mcp2515_t *pi_mcp2515, uint8_t reqop)
{
	mcp2515_register_bitmod(pi_mcp2515, reqop, PI_MCP2515_REQOP_MASK, PI_MCP2515_RGSTR_CANCTRL);
}

void
mcp2515_init(pi_mcp2515_t *pi_mcp2515, spi_inst_t *spi_channel, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin,
		uint8_t sck_pin, uint32_t spi_clock)
{
	pi_mcp2515->spi_channel = spi_channel;
	pi_mcp2515->cs_pin = cs_pin;
	pi_mcp2515->sck_pin = sck_pin;
	pi_mcp2515->tx_pin = tx_pin;
	pi_mcp2515->rx_pin = rx_pin;
	pi_mcp2515->spi_clock = spi_clock;

	spi_init(spi_channel, spi_clock);
	gpio_set_function(tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(rx_pin, GPIO_FUNC_SPI);
	gpio_set_function(sck_pin, GPIO_FUNC_SPI);

	gpio_init(cs_pin);
	gpio_set_dir(cs_pin, GPIO_OUT);

	UNSET_CS(pi_mcp2515);
}
