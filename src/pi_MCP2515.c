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

/* Warning, this is still an early work in progress.
 * There is a lot of bad/untested code here. Please be patient, I'm working on it...
 */

#include <stdbool.h>
#include <string.h>

#include "gpio.h"

#include "pi_MCP2515.h"

static const uint8_t tx_reg_list[][2] = {
	/* CTRL, SIDH */
	{ 0x30, 0x31 },
	{ 0x40, 0x41 },
	{ 0x50, 0x51 }
};

int
mcp2515_can_message_send(pi_mcp2515_t *pi_mcp2515, const pi_mcp2515_can_frame_t *can_frame)
{
	int res;
	uint16_t id_tmp;
	uint8_t payload[13], ctrl;
	bool extended_id;

	res = -1;

	for (int i = 0; i < (sizeof(tx_reg_list) / sizeof(tx_reg_list[0])); i++) {
		mcp2515_register_read(pi_mcp2515, &ctrl, 1, tx_reg_list[i][0]);
		if (ctrl & PI_MCP2515_CTRL_TXREQ) {
			extended_id = can_frame->id & PI_MCP2515_FLAG_EFF;

			id_tmp = (uint16_t)((can_frame->id & (extended_id ? PI_MCP2515_ID_MASK_EFF
				: PI_MCP2515_ID_MASK_SFF)) & 0x0FFFF);
			if (extended_id) {
				payload[3] = (uint8_t)(id_tmp & 0xFF);
				payload[2] = (uint8_t)(id_tmp >> 8);
				id_tmp = (uint16_t)(id_tmp >> 16);
				payload[1] = ((uint8_t)(id_tmp & 0x03) + (uint8_t)((id_tmp & 0x1C) << 3)) | 0x08;
				payload[0] = (uint8_t)(id_tmp >> 5);
			} else {
				payload[0] = (uint8_t)(id_tmp >> 3);
				payload[1] = (uint8_t)((id_tmp & 0x07) << 5);
				payload[2] = 0;
				payload[3] = 0;
			}

			memcpy(&payload[4], can_frame->payload, can_frame->dlc);
			mcp2515_register_write(pi_mcp2515, payload, can_frame->dlc + 5, tx_reg_list[i][1]);
			mcp2515_register_bitmod(pi_mcp2515, PI_MCP2515_CTRL_TXREQ, PI_MCP2515_CTRL_TXREQ,
				tx_reg_list[i][1]);

			/* Check status again for errors */
			mcp2515_register_read(pi_mcp2515, &ctrl, 1, tx_reg_list[i][0]);
			if (ctrl & (PI_MCP2515_CTRL_TXERR| PI_MCP2515_CTRL_MLOA | PI_MCP2515_CTRL_ABTF)) {
				res = 1;
				break;
			}
		}
	}

	return (res);
}

int
mcp2515_can_message_read(pi_mcp2515_t *pi_mcp2515, pi_mcp2515_can_frame_t *can_frame)
{
	uint32_t id;
	int res;
	uint8_t buffer[5], status, dlc, ctrl, ctrl_reg, sidh_reg, data_reg, canintf;

	res = 0;

	/* TODO There is a lot of clutter and hardcoded values. This should be more tidy. */
	SET_CS(pi_mcp2515);
	status = mcp2515_status(pi_mcp2515);
	if (status & PI_MCP2515_STATUS_RX0BF) {
		ctrl_reg = PI_MCP2515_RGSTR_RX0CTRL;
		sidh_reg = 0x61;
		data_reg = 0x66;
		canintf = 0x01;
	} else if (status & PI_MCP2515_STATUS_RX1BF) {
		ctrl_reg = PI_MCP2515_RGSTR_RX1CTRL;
		sidh_reg = 0x71;
		data_reg = 0x76;
		canintf = 0x02;
	} else {
		res = -1;
		goto end;
	}
	mcp2515_register_read(pi_mcp2515, buffer, 5, sidh_reg);

	id = (buffer[0] << 3) | (buffer[1] >> 5);
	if (buffer[1] & 0x08) { /* Uses expanded ID */
		id = (((((id << 2) + (buffer[1] & 0x03)) << 8) + buffer[2]) << 8) + buffer[3];
	}
	dlc = buffer[4] & 0x0F;

	mcp2515_register_read(pi_mcp2515, &ctrl, 1, ctrl_reg);
	if (ctrl & PI_MCP2515_CTRL_RTR) {
		id |= PI_MCP2515_FLAG_RTR;
	}

	can_frame->id = id;
	can_frame->dlc = dlc;

	mcp2515_register_read(pi_mcp2515, can_frame->payload, dlc, data_reg);
	mcp2515_register_bitmod(pi_mcp2515, 0, canintf, PI_MCP2515_RGSTR_CANINTF);
end:
	UNSET_CS(pi_mcp2515);
	return (res);
}

uint8_t
mcp2515_status(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t instruction, res;

	SET_CS(pi_mcp2515);
	instruction = PI_MCP2515_INSTR_READ_STATUS;
	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instruction, 1);
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, &res, 1);
	UNSET_CS(pi_mcp2515);

	return (res);
}

int
mcp2515_register_read(pi_mcp2515_t *pi_mcp2515, uint8_t data[], uint8_t len, uint8_t rgstr)
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

int
mcp2515_reqop(pi_mcp2515_t *pi_mcp2515, uint8_t reqop)
{
	return (mcp2515_register_bitmod(pi_mcp2515, reqop, PI_MCP2515_REQOP_MASK,
		PI_MCP2515_RGSTR_CANCTRL));
}

/* TODO These defaults are untested. Make sure this all makes sense. */
int
mcp2515_bitrate_default_16mhz_1000kbps(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_bitrate_simplified(pi_mcp2515, 1000, 16));
}

int
mcp2515_bitrate_default_8mhz_500kbps(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_bitrate_simplified(pi_mcp2515, 500, 8));
}

int
mcp2515_bitrate_simplified(pi_mcp2515_t *pi_mcp2515, uint16_t baudrate_kbps, uint8_t osc_mhz)
{
	return (mcp2515_bitrate_full_optional(pi_mcp2515, baudrate_kbps, osc_mhz, 2, 0, 2,
		2, 3, false, false, false, true));
}

/* TODO first draft. Test/validate/fix. Make sure this is right. */
int
mcp2515_bitrate_full_optional(pi_mcp2515_t *pi_mcp2515, uint16_t baudrate_kbps, uint8_t osc_mhz, uint8_t sjw,
	uint8_t prescaler, uint8_t prseg_tqps, uint8_t phseg_tqps1, uint8_t phseg_tqps2, bool sof, bool wakfil,
	bool sam, bool btlmode)
{
	uint8_t cnf1, cnf2, cnf3;

	if (baudrate_kbps > 1000 || baudrate_kbps == 0 || osc_mhz > 40|| osc_mhz == 0 || sjw > 4 || sjw == 0
			|| prescaler > 63 || prseg_tqps == 0 || prseg_tqps > 8 || phseg_tqps1 == 0 || phseg_tqps1 > 8
			|| phseg_tqps2 == 0 || phseg_tqps2 > 8 || phseg_tqps2 <= sjw
			|| prseg_tqps + phseg_tqps1 < phseg_tqps2) {
		return (1);
	}

	cnf1 = ((sjw - 1) << 6) | (prescaler & 0x3f);
	cnf2 = (((phseg_tqps1 - 1) & 0x07) << 3) | (prseg_tqps & 0x7);
	cnf3 = (phseg_tqps2 - 1) & 0x07;

	if (btlmode) {
		cnf2 |= 0x80;
	}
	if (sam) {
		cnf2 |= 0x40;
	}
	if (sof) {
		cnf3 |= 0x80;
	}
	if (wakfil) {
		cnf3 |= 0x40;
	}

	mcp2515_register_write(pi_mcp2515, &cnf1, 1, PI_MCP2515_RGSTR_CNF1);
	mcp2515_register_write(pi_mcp2515, &cnf2, 1, PI_MCP2515_RGSTR_CNF2);
	mcp2515_register_write(pi_mcp2515, &cnf3, 1, PI_MCP2515_RGSTR_CNF3);

	return (0);
}

int
mcp2515_reset(pi_mcp2515_t *pi_mcp2515)
{
	int res;
	uint8_t instr = PI_MCP2515_INSTR_RESET, blank[14] = { 0 };

	SET_CS(pi_mcp2515);
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
	UNSET_CS(pi_mcp2515);

	if (res)
		return (res);

	/* TODO wait? */

	/* Return a sum of all return values. This is a reset function, so try and reset all registers. */
	for (int i = 0; i < sizeof (tx_reg_list) / sizeof (tx_reg_list[0]); i++)
		res += mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), tx_reg_list[i][0]);
	res += mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX0CTRL);
	res += mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX1CTRL);

	return (res);
}

uint8_t
mcp2515_error_tx_count(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t res = 0;

	mcp2515_register_read(pi_mcp2515, &res, 1, PI_MCP2515_RGSTR_ECTX);

	return (res);
}

uint8_t
mcp2515_error_rx_count(pi_mcp2515_t *pi_mcp2515)
{
	uint8_t res = 0;

	mcp2515_register_read(pi_mcp2515, &res, 1, PI_MCP2515_RGSTR_ECRX);

	return (res);
}

int
mcp2515_init(pi_mcp2515_t *pi_mcp2515, uint8_t spi_channel, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin,
		uint8_t sck_pin, uint32_t spi_clock)
{
	int res;

	pi_mcp2515->spi_channel = spi_channel;
	pi_mcp2515->cs_pin = cs_pin;
	pi_mcp2515->sck_pin = sck_pin;
	pi_mcp2515->tx_pin = tx_pin;
	pi_mcp2515->rx_pin = rx_pin;
	pi_mcp2515->spi_clock = spi_clock;

	if ((res = mcp2515_gpio_spi_init(pi_mcp2515, spi_channel, spi_clock)))
		return (res);

	UNSET_CS(pi_mcp2515);
	return (0);
}

void
mcp2515_free(pi_mcp2515_t *pi_mcp2515)
{
	mcp2515_gpio_spi_free(pi_mcp2515);
	/* TODO incomplete */
}
