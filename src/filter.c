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

#include "pi_MCP2515_defs.h"
#include "pi_MCP2515_handle.h"
#include "registers.h"

#include "filter.h"

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
 * @defgroup piMCP2515_filter_functions Filter Functions
 * @brief These functions handle filter functionality.
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