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

#ifdef USE_PICO_LIB
#define MICRO_SLEEP(x) sleep_us(x)
#else
#include <unistd.h>
#define MICRO_SLEEP(x) usleep(x);
#endif

/* A map of CTRL and SIDH TX registers. This is used by `mcp2515_can_message_send` to find an available TX buffer, and
 * by `mcp2515_reset` to iterate over all TX buffers.
 */
static const uint8_t tx_reg_list[][2] = {
	/* CTRL, SIDH */
	{ 0x30, 0x31 },
	{ 0x40, 0x41 },
	{ 0x50, 0x51 }
};

/**
 * @brief Assemble a CAN bus message frame ID.
 *
 * @param id the ID to assemble
 * @param extended_id if it is using an extended ID
 * @return 4 bytes representing the ID as a uint32_t.
 */
inline static uint32_t
id_build(uint32_t id, bool extended_id)
{
	uint16_t id_tmp = 0;
	uint8_t result[4] = {0};

	id_tmp = (uint16_t)((id & (extended_id ? PI_MCP2515_ID_MASK_EFF : PI_MCP2515_ID_MASK_SFF)) & 0x0FFFF);
	if (extended_id) {
		result[3] = (uint8_t)(id_tmp & 0xFF);
		result[2] = (uint8_t)(id_tmp >> 8);
		id_tmp = (uint16_t)(id_tmp >> 16);
		result[1] = ((uint8_t)(id_tmp & 0x03) + (uint8_t)((id_tmp & 0x1C) << 3)) | 0x08;
		result[0] = (uint8_t)(id_tmp >> 5);
	} else {
		result[0] = (uint8_t)(id_tmp >> 3);
		result[1] = (uint8_t)((id_tmp & 0x07) << 5);
		result[2] = 0;
		result[3] = 0;
	}

	return (*((uint32_t *)result));
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


/**
 * @defgroup piMCP2515_filter_functions Filter Functions
 * @brief These functions handle filter functionallity.
 * @{
 */
/**
 * @brief Set one of the filters specified by its register.
 *
 * The MCP2515 must be in config mode to use this (see mcp2515_reqop
 * and mcp2515_reqop_get).
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param filter_reg the filter register.
 *                   This may be any of the following:
 *                   - PI_MCP2515_RGSTR_RXF0SIDH
 *                   - PI_MCP2515_RGSTR_RXF1SIDH
 *                   - PI_MCP2515_RGSTR_RXF2SIDH
 *                   - PI_MCP2515_RGSTR_RXF3SIDH
 *                   - PI_MCP2515_RGSTR_RXF4SIDH
 *                   - PI_MCP2515_RGSTR_RXF5SIDH
 * @param id
 * @param extended_id
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter(pi_mcp2515_t *pi_mcp2515, uint8_t filter_reg, uint32_t id, bool extended_id)
{
	int res;
	uint32_t data;

	data = id_build(id, extended_id);
	res = mcp2515_register_write(pi_mcp2515, (uint8_t *)&data, 4, filter_reg);

	return (res);
}

/**
 * @brief Set the filter mask. The MCP2515 must be in config mode to use this (see mcp2515_reqop/mcp2515_reqop_get).
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param filter_mask_reg the filter mask register to use.
 *                        Acceptable values are PI_MCP2515_RGSTR_RXM0SIDH and PI_MCP2515_RGSTR_RXM1SIDH.
 * @param id_mask the ID mask to use.
 * @param extended_id if the ID is extended.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter_mask(pi_mcp2515_t *pi_mcp2515, uint8_t filter_mask_reg, int32_t id_mask, bool extended_id)
{
	int res;
	uint32_t data;

	data = id_build(id_mask, extended_id);
	res = mcp2515_register_write(pi_mcp2515, (uint8_t *)&data, 4, filter_mask_reg);

	return (res);
}
/** @} */

/**
 * @defgroup piMCP2515_mode_error_status_functions Mode/Error/Status Functions
 * @brief These functions handle REQOP (Operating Mode), Status, and Errors.
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

	SET_CS(pi_mcp2515);
	instruction = PI_MCP2515_INSTR_READ_STATUS;
	mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instruction, 1);
	mcp2515_gpio_spi_read_blocking(pi_mcp2515, &res, 1);
	UNSET_CS(pi_mcp2515);

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
	MICRO_SLEEP(mcp2515_osc_time(pi_mcp2515, 128));

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

	return (reqop & PI_MCP2515_REQOP_MASK_CANSTAT);
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
/** @} */


/**
 * @defgroup piMCP2515_config_init_functions Init and Config Functions
 * @brief These functions handle initialization and configuration.
 * @{
 */
/**
 * @brief A simplified bitrate setting function.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param baud_rate_kbps the baud rate to use in kbps. Commonly either 500 or 1000.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_bitrate_simplified(pi_mcp2515_t *pi_mcp2515, uint16_t baud_rate_kbps)
{
	return (mcp2515_bitrate_full_optional(pi_mcp2515, baud_rate_kbps, 2, 0, 2, 2,
	    3, false, false, false, true));
}

/**
 * @brief Configure the bitrate and all other CNF register stored parameters.
 *
 * This function accepts all possible configurable
 * options, while `mcp2515_bitrate_simplified` can be used for a simplified process.
 *
 * TODO first draft. Test/validate/fix. Make sure this is right.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param baud_rate_kbps the baud rate to use in kbps. Commonly either 500 or 1000.
 * @param sjw the synchronization jump width in Tqs (1-4).
 * @param prescaler the prescaler divisor (0-63)
 * @param prseg_tqps the prop segment Tqs/segment
 * @param phseg_tqps1 the phase segment 1 Tqs/segment
 * @param phseg_tqps2 the phase segment 2 Tqs/segment
 * @param sof if start-of-frame signal is enabled.
 * @param wakfil if wakeup filter should be enabled.
 * @param sam is bus line should be sampled 3 times at sample point (otherwise it is sampled once).
 * @param btlmode if BTLMODE is enabled.
 *                true:  PS2 length is determined by PHSEG2[2:0] (PHSEG2 values originate from @p phseg_tqps2).
 *                false: PS2 length is the greater of PS1 and IPT (2 Tqs)
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_bitrate_full_optional(pi_mcp2515_t *pi_mcp2515, uint16_t baud_rate_kbps, uint8_t sjw, uint8_t prescaler,
    uint8_t prseg_tqps, uint8_t phseg_tqps1, uint8_t phseg_tqps2, bool sof, bool wakfil, bool sam, bool btlmode)
{
	int res;
	uint8_t cnf1, cnf2, cnf3;

	res = 0;

	if (baud_rate_kbps > 1000 || baud_rate_kbps == 0 || sjw > 4 || sjw == 0 || prescaler > 63 || prseg_tqps == 0
	    || prseg_tqps > 8 || phseg_tqps1 == 0 || phseg_tqps1 > 8 || phseg_tqps2 == 0 || phseg_tqps2 > 8
	    || phseg_tqps2 <= sjw || prseg_tqps + phseg_tqps1 < phseg_tqps2) {
		res = 1;
		goto err;
	}

	cnf1 = ((sjw - 1) << 6) | (prescaler & 0x3f);
	cnf2 = (((phseg_tqps1 - 1) & 0x07) << 3) | (prseg_tqps & 0x7);
	cnf3 = (phseg_tqps2 - 1) & 0x07;

	if (btlmode)
		cnf2 |= 0x80;
	if (sam)
		cnf2 |= 0x40;
	if (sof)
		cnf3 |= 0x80;
	if (wakfil)
		cnf3 |= 0x40;

	if ((res = mcp2515_register_write(pi_mcp2515, &cnf1, 1, PI_MCP2515_RGSTR_CNF1)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, &cnf2, 1, PI_MCP2515_RGSTR_CNF2)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, &cnf3, 1, PI_MCP2515_RGSTR_CNF3);

err:
	return (res);
}

/**
 * @brief Set the values of all CNF register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param cnf1 the value to use for the CNF1 register.
 * @param cnf2 the value to use for the CNF2 register.
 * @param cnf3 the value to use for the CNF3 register.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_cnf_set(pi_mcp2515_t *pi_mcp2515, uint8_t cnf1, uint8_t cnf2, uint8_t cnf3)
{
	int res;

	if ((res = mcp2515_register_write(pi_mcp2515, &cnf1, 1, PI_MCP2515_RGSTR_CNF1)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, &cnf2, 1, PI_MCP2515_RGSTR_CNF2)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, &cnf3, 1, PI_MCP2515_RGSTR_CNF3);

	err:
		return (res);
}

/**
 * @brief Get the value of a CNF register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param cnf The CNF register index such that `cnf` being `1` will fetch from CNF1, etc.
 * @return The value of the register.
 */
uint8_t
mcp2515_cnf_get(pi_mcp2515_t *pi_mcp2515, uint8_t cnf)
{
	uint8_t res = 0, rgstr;

	/* CNF register numbers aren't zero indexed for some indiscernible reason. Start with 1 here for consistency. */
	switch (cnf) {
	case 1:
		rgstr = PI_MCP2515_RGSTR_CNF1;
		break;
	case 2:
		rgstr = PI_MCP2515_RGSTR_CNF2;
		break;
	case 3:
		rgstr = PI_MCP2515_RGSTR_CNF3;
		break;
	default:
		goto err;
	}

	mcp2515_register_read(pi_mcp2515, &res, 1, rgstr);

err:
	return (res);
}

/**
 * @brief Issue a reset command to the MCP2515 via the SPI interface.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_reset(pi_mcp2515_t *pi_mcp2515)
{
	int res, i;
	uint8_t instr = PI_MCP2515_INSTR_RESET, blank[14] = { 0 };

	SET_CS(pi_mcp2515);
	res = mcp2515_gpio_spi_write_blocking(pi_mcp2515, &instr, 1);
	UNSET_CS(pi_mcp2515);

	if (res)
		goto err;

	MICRO_SLEEP(mcp2515_osc_time(pi_mcp2515, 128));

	for (i = 0; i < sizeof (tx_reg_list) / sizeof (tx_reg_list[0]); i++)
		if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), tx_reg_list[i][0])))
			goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX0CTRL)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, blank, sizeof(blank), PI_MCP2515_RGSTR_RX1CTRL);

err:
	return (res);
}

/**
 * @brief Setup and prepare a pi_mcp2515_t structure based on the parameters provided.
 *
 * This will still require one of the bitrate setting functions or mcp2515_cnf_set to be used to set up the CNF
 * registers.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 *
 * @param osc_mhz the frequency of the oscillator in MHz.
 * @param spi_channel the SPI channel to use which may be `0` or `1`.
 * @param cs_pin the GPIO pin to use for SPI chip select.
 * @return zero if success, otherwise non-zero
 */
int
mcp2515_init(pi_mcp2515_t *pi_mcp2515, uint8_t spi_channel, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin,
    uint8_t sck_pin, uint32_t spi_clock, uint8_t osc_mhz)
{
	int res = 0;

	if (osc_mhz > 40|| osc_mhz == 0) {
		res = 1;
		goto err;
	}

	pi_mcp2515->spi_channel = spi_channel;
	pi_mcp2515->cs_pin = cs_pin;
	pi_mcp2515->sck_pin = sck_pin;/* TODO this is only used for Pico. Figure out a cleaner way. */
	pi_mcp2515->tx_pin = tx_pin; /* TODO this is only used for Pico. Figure out a cleaner way. */
	pi_mcp2515->rx_pin = rx_pin; /* TODO this is only used for Pico. Figure out a cleaner way. */
	pi_mcp2515->spi_clock = spi_clock; /* TODO this is only used for spidev. Figure out a cleaner way. */
	pi_mcp2515->osc_mhz = osc_mhz;

	if ((res = mcp2515_gpio_spi_init(pi_mcp2515, spi_channel, spi_clock)))
		goto err;

	UNSET_CS(pi_mcp2515);

err:
	return (res);
}

/**
 * @brief Calculate the time for the number of oscillator cycles supplied, and based on the oscillator frequency.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param num_cycles The number of oscillator cycles to calculate time for.
 * @return The calculated time in microseconds.
 */
uint64_t
mcp2515_osc_time(const pi_mcp2515_t *pi_mcp2515, uint32_t num_cycles)
{
	uint64_t cycle_len_nano_sec;

	cycle_len_nano_sec = 1000000000 / (pi_mcp2515->osc_mhz * 1000000);

	return (num_cycles * cycle_len_nano_sec / 1000); /* return microseconds */
}

/**
 * @brief Cleanup after everything.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 */
void
mcp2515_free(const pi_mcp2515_t *pi_mcp2515)
{
	mcp2515_gpio_spi_free(pi_mcp2515);
}
/** @} */
