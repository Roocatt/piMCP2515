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

#include <string.h>

#include "../include/pi_MCP2515_defs.h"

#include "debug.h"
#include "pi_MCP2515_handle.h"
#include "registers.h"
#include "gpio.h"
#include "status_error.h"

#include "can.h"

static const uint8_t tx_reg_list[][2] = {
	/* CTRL, INSTR */
	{ 0x30, PI_MCP2515_INSTR_LOAD_TX0 },
	{ 0x40, PI_MCP2515_INSTR_LOAD_TX1 },
	{ 0x50, PI_MCP2515_INSTR_LOAD_TX2 }
};

/**
 * @defgroup piMCP2515_can_functions CAN Bus Functions
 * @brief These functions handle CAN bus functionality.
 * @{
 */
/**
 * @brief Send a CAN bus message.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param can_frame the CAN bus frame to send.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_can_message_send(pi_mcp2515_t *pi_mcp2515, const pi_mcp2515_can_frame_t *can_frame)
{
	int res;
	uint16_t id_tmp;
	uint8_t payload[13], ctrl, instr, i;
	bool extended_id, rtr;

	res = -1;

	for (i = 0; i < (uint8_t)(sizeof(tx_reg_list) / sizeof(tx_reg_list[0])); i++) {
		mcp2515_register_read(pi_mcp2515, &ctrl, 1, tx_reg_list[i][0]);
		MCP2515_DEBUG(pi_mcp2515, "checking tx_reg_list[%d] CTRL: 0x%02x\n", ctrl);
		if ((ctrl & PI_MCP2515_CTRL_TXREQ) == 0) {
			MCP2515_DEBUG(pi_mcp2515, "Using tx_reg_list[%d]\n", i);
			extended_id = !!(can_frame->id & PI_MCP2515_FLAG_EFF);
			rtr = !!(can_frame->id & PI_MCP2515_FLAG_RTR);

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

			payload[4] = rtr ? (can_frame->dlc | PI_MCP2515_RTR_MASK) : can_frame->dlc;
			memcpy(&payload[4], can_frame->payload, can_frame->dlc);
			instr = tx_reg_list[i][1];
			mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
			mcp2515_gpio_spi_write_blocking(pi_mcp2515, payload, can_frame->dlc + 5);
			res = 0;
			break;
		}
	}
	if (res != -1) {
		mcp2515_rts(pi_mcp2515, i);

		/* Check status again for errors */
		mcp2515_register_read(pi_mcp2515, &ctrl, 1, tx_reg_list[i][0]);
		if (ctrl & (PI_MCP2515_CTRL_TXERR | PI_MCP2515_CTRL_MLOA | PI_MCP2515_CTRL_ABTF)) {
			MCP2515_DEBUG(pi_mcp2515, "TX%dCTRL has errors. Value: 0x%02x\n", ctrl);
			res = 1;
		}
	} else
		MCP2515_DEBUG(pi_mcp2515, "no available tx found\n");

	return (res);
}

/**
 * @brief Read a CAN bus message.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param can_frame a pointer to the structure to store the received CAN bus frame.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_can_message_read(pi_mcp2515_t *pi_mcp2515, pi_mcp2515_can_frame_t *can_frame)
{
	uint32_t id;
	int res;
	uint8_t buffer[5], status, dlc, ctrl, ctrl_reg, sidh_reg, data_reg, canintf;

	res = 0;

	SET_CS(pi_mcp2515);

	status = mcp2515_status(pi_mcp2515);
	if (status & PI_MCP2515_STATUS_RX0BF) {
		ctrl_reg = PI_MCP2515_RGSTR_RX0CTRL;
		sidh_reg = PI_MCP2515_RGSTR_RX0SIDH;
		data_reg = PI_MCP2515_RGSTR_RX0DATA;
		canintf = PI_MCP2515_CANINTF_RX0;
	} else if (status & PI_MCP2515_STATUS_RX1BF) {
		ctrl_reg = PI_MCP2515_RGSTR_RX1CTRL;
		sidh_reg = PI_MCP2515_RGSTR_RX1SIDH;
		data_reg = PI_MCP2515_RGSTR_RX1DATA;
		canintf = PI_MCP2515_CANINTF_RX1;
	} else {
		res = -1;
		goto end;
	}
	mcp2515_register_read(pi_mcp2515, buffer, 5, sidh_reg);

	id = (buffer[0] << 3) | (buffer[1] >> 5);

	if (buffer[1] & 0x08) /* Uses expanded ID */
		id = (((((id << 2) + (buffer[1] & 0x03)) << 8) + buffer[2]) << 8) + buffer[3];

	dlc = buffer[4] & 0x0F;

	mcp2515_register_read(pi_mcp2515, &ctrl, 1, ctrl_reg);
	if (ctrl & PI_MCP2515_CTRL_RTR)
		id |= PI_MCP2515_FLAG_RTR;

	can_frame->id = id;
	can_frame->dlc = dlc;

	mcp2515_register_read(pi_mcp2515, can_frame->payload, dlc, data_reg);
	mcp2515_register_bitmod(pi_mcp2515, 0, canintf, PI_MCP2515_RGSTR_CANINTF);

end:
	UNSET_CS(pi_mcp2515);

	return (res);
}

/**
 * @brief Check if there is a CAN bus message received.
 *
 * The message can then be read with `mcp2515_can_message_read`.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return true if a message is received in one of the RX buffers.
 */
bool
mcp2515_can_message_received(pi_mcp2515_t *pi_mcp2515)
{
	return (!!(mcp2515_status(pi_mcp2515) & (PI_MCP2515_STATUS_RX0BF | PI_MCP2515_STATUS_RX0BF)));
}
/** @} */

int
mcp2515_rts(pi_mcp2515_t *pi_mcp2515, uint8_t buffer)
{
	uint8_t instruction, res = -1;

	SET_CS(pi_mcp2515);
	switch (buffer) {
	case 0:
		instruction = PI_MCP2515_INSTR_RTS_TX0;
		break;
	case 1:
		instruction = PI_MCP2515_INSTR_RTS_TX1;
		break;
	case 2:
		instruction = PI_MCP2515_INSTR_RTS_TX2;
		break;
	default:
		goto end;
	}

	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instruction, 1);
	UNSET_CS(pi_mcp2515);

	MCP2515_DEBUG(pi_mcp2515, "MCP2515 RTS %04x\n", res);
end:
	return (res);
}