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

#include "pi_MCP2515_handle.h"
#include "../include/pi_MCP2515_defs.h"
#include "registers.h"

#include "can.h"

#include "gpio.h"
#include "status_error.h"

static const uint8_t tx_reg_list[][2] = {
	/* CTRL, SIDH */
	{ 0x30, 0x31 },
	{ 0x40, 0x41 },
	{ 0x50, 0x51 }
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
	int res, i;
	uint16_t id_tmp;
	uint8_t payload[13], ctrl;
	bool extended_id;

	res = -1;

	for (i = 0; i < (sizeof(tx_reg_list) / sizeof(tx_reg_list[0])); i++) {
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