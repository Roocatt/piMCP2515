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

#include <stdio.h>

static const uint8_t tx_reg_list[][3] = {
	/* CTRL, INSTR, TBxIF CANINTF flag */
	{ 0x30, PI_MCP2515_INSTR_LOAD_TX0, PI_MCP2515_CANINTF_TX0IF },
	{ 0x40, PI_MCP2515_INSTR_LOAD_TX1, PI_MCP2515_CANINTF_TX1IF },
	{ 0x50, PI_MCP2515_INSTR_LOAD_TX2, PI_MCP2515_CANINTF_TX2IF }
};

/**
 * @brief Assemble a CAN bus message frame ID.
 *
 * @param id the ID to assemble
 * @param extended_id if it is using an extended ID
 * @return 4 bytes representing the ID as a uint32_t.
 */
uint32_t
mcp2515_can_id_build(uint32_t id, bool extended_id)
{
	uint16_t id_tmp = 0;
	uint8_t result[4] = {0};

	id = id & (extended_id ? PI_MCP2515_CAN_ID_EFF_MASK : PI_MCP2515_CAN_ID_SFF_MASK);

	id_tmp = (uint16_t)(id & 0x0000FFFF);
	if (extended_id) {
		result[3] = (uint8_t)(id_tmp & 0x00FF);
		result[2] = (uint8_t)(id_tmp >> 8);
		id_tmp = (uint16_t)(id >> 16);
		result[1] = (((uint8_t)(id_tmp & 0x03)) + ((uint8_t)((id_tmp & 0x1C) << 3))) | 0x08;
		result[0] = (uint8_t)(id_tmp >> 5);
	} else {
		result[3] = 0;
		result[2] = 0;
		result[1] = (uint8_t)((id_tmp & 0x07) << 5);
		result[0] = (uint8_t)(id_tmp >> 3);
	}

	return (*((uint32_t *)result));
}

/**
 * @brief Clear a TX buffer empty interrupt flag.
 *
 * Sending a message will set one of these flags. Use this function to clear it.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param index which TX buffer empty interrupt flag to clear (0-2).
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_can_clear_txif(pi_mcp2515_t *pi_mcp2515, const uint8_t index)
{
	int res;
	uint8_t flag;

	switch (index) {
	case 0:
		flag = PI_MCP2515_CANINTF_TX0IF;
		break;
	case 1:
		flag = PI_MCP2515_CANINTF_TX1IF;
		break;
	case 2:
		flag = PI_MCP2515_CANINTF_TX2IF;
		break;
	default:
		res = -1;
		goto end;
	}
	MCP2515_DEBUG(pi_mcp2515, "clearing TX%dIF\n", index);
	CS_LOW(pi_mcp2515);
	res = mcp2515_register_bitmod(pi_mcp2515, 0, flag, PI_MCP2515_RGSTR_CANINTF);
	CS_HIGH(pi_mcp2515);
end:
	return (res);
}

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
	uint32_t built_id;
	uint8_t payload[13], ctrl, instr, canintf, i;
	bool extended_id, rtr;

	res = -1;

	for (i = 0; i < (uint8_t)(sizeof(tx_reg_list) / sizeof(tx_reg_list[0])); i++) {
		mcp2515_register_read(pi_mcp2515, &ctrl, 1, tx_reg_list[i][0]);
		MCP2515_DEBUG(pi_mcp2515, "checking tx_reg_list[%d] CTRL: 0x%02x\n", ctrl);
		if ((ctrl & PI_MCP2515_CTRL_TXREQ) == 0) {
			MCP2515_DEBUG(pi_mcp2515, "Using tx_reg_list[%d]\n", i);
			extended_id = !!(can_frame->id & PI_MCP2515_FLAG_EFF);
			rtr = !!(can_frame->id & PI_MCP2515_FLAG_RTR);

			built_id = mcp2515_can_id_build(can_frame->id, extended_id);
			*((uint32_t *)payload) = built_id;

			payload[4] = rtr ? (can_frame->dlc | PI_MCP2515_CAN_DLC_RTR_MASK) : can_frame->dlc;
			if (!rtr)
				memcpy(&payload[5], can_frame->payload, can_frame->dlc);

			instr = tx_reg_list[i][1];
			CS_LOW(pi_mcp2515);
			mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
			mcp2515_gpio_spi_write_blocking(pi_mcp2515, payload, rtr ? 5 : (can_frame->dlc + 5));
			CS_HIGH(pi_mcp2515);

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

		mcp2515_register_read(pi_mcp2515, &canintf, 1, PI_MCP2515_RGSTR_CANINTF);
		if ((canintf | tx_reg_list[i][2]) == 0) {
			res = 1;
			MCP2515_DEBUG(pi_mcp2515, "TXxIF not set after sending.\n");
		}
		mcp2515_can_clear_txif(pi_mcp2515, i);
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
	uint8_t buffer[10], status, dlc, instr;
	bool rtr, extended_id;

	res = 0;
	instr = PI_MCP2515_INSTR_RX_STATUS;

	CS_LOW(pi_mcp2515);

	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, &status, 1);

	if (status & PI_MCP2515_RX_STATUS_RCV_RXB0) {
		instr = PI_MCP2515_INSTR_READ_RX0;
	} else if (status & PI_MCP2515_RX_STATUS_RCV_RXB1) {
		instr = PI_MCP2515_INSTR_READ_RX1;
	} else {
		res = -1;
		goto end;
	}
	CS_HIGH(pi_mcp2515);

	/* The MCP2515 prioritizes RXB0, so if both PI_MCP2515_RX_STATUS_RCV_RXB0 and PI_MCP2515_RX_STATUS_RCV_RXB1 are
	 * set in status, indicating both buffers used, then the bits of status masked by 0x1f only apply to RXB0. As
	 * RXB1 is only checked when RXB0 is empty in the above if/else, this does not present a problem.
	 */
	extended_id = !!(status & PI_MCP2515_RX_STATUS_EID);
	rtr = !!(status & PI_MCP2515_RX_STATUS_RTR);

	CS_LOW(pi_mcp2515);
	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, buffer, 5);

	id = ((uint16_t)buffer[0] << 3) | (buffer[1] >> 5);
	if (extended_id)
		id = (((((id << 2) + (buffer[1] & 0x03)) << 8) + buffer[2]) << 8) + buffer[3];
	dlc = buffer[4] & 0x0F;

	if (rtr)
		id |= PI_MCP2515_FLAG_RTR;

	can_frame->id = id;
	can_frame->dlc = dlc;
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, can_frame->payload, dlc);

end:
	CS_HIGH(pi_mcp2515);

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

void
mcp2515_rts(pi_mcp2515_t *pi_mcp2515, uint8_t buffer)
{
	uint8_t instruction;

	CS_LOW(pi_mcp2515);
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
		return;
	}

	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instruction, 1);
	CS_HIGH(pi_mcp2515);
}