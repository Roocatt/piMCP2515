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

#include "../include/pi_MCP2515_defs.h"
#include "pi_MCP2515_handle.h"
#include "registers.h"
#include "can.h"

#include "filter.h"

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
 * @param filter_index the filter register.
 *                   This may be any of the following:
 *                   - 0 for PI_MCP2515_RGSTR_RXF0SIDH
 *                   - 1 for PI_MCP2515_RGSTR_RXF1SIDH
 *                   - 2 for PI_MCP2515_RGSTR_RXF2SIDH
 *                   - 3 for PI_MCP2515_RGSTR_RXF3SIDH
 *                   - 4 for PI_MCP2515_RGSTR_RXF4SIDH
 *                   - 5 for PI_MCP2515_RGSTR_RXF5SIDH
 * @param id
 * @param extended_id
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter(pi_mcp2515_t *pi_mcp2515, uint8_t filter_index, uint32_t id, bool extended_id)
{
	int res;
	uint32_t data;
	uint8_t filter_mask_reg;

	switch (filter_index) {
	case 0:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF0SIDH;
		break;
	case 1:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF1SIDH;
		break;
	case 2:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF2SIDH;
		break;
	case 3:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF3SIDH;
		break;
	case 4:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF4SIDH;
		break;
	case 5:
		filter_mask_reg = PI_MCP2515_RGSTR_RXF5SIDH;
		break;
	default:
		res = -1;
		goto end;
	}

	data = mcp2515_can_id_build(id, extended_id);
	res = mcp2515_register_write(pi_mcp2515, (uint8_t *)&data, 4, filter_mask_reg);

end:
	return (res);
}

/**
 * @brief Set the filter mask. The MCP2515 must be in config mode to use this (see mcp2515_reqop/mcp2515_reqop_get).
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param filter_mask_index the filter mask register to use.
 *                        Acceptable values are 0 for PI_MCP2515_RGSTR_RXM0SIDH and 1 for PI_MCP2515_RGSTR_RXM1SIDH.
 * @param id_mask the ID mask to use.
 * @param extended_id if the ID is extended.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter_mask(pi_mcp2515_t *pi_mcp2515, uint8_t filter_mask_index, uint32_t id_mask, bool extended_id)
{
	int res;
	uint32_t data;
	uint8_t filter_mask_reg;

	switch (filter_mask_index) {
	case 0:
		filter_mask_reg = PI_MCP2515_RGSTR_RXM0SIDH;
		break;
	case 1:
		filter_mask_reg = PI_MCP2515_RGSTR_RXM1SIDH;
		break;
	default:
		res = -1;
		goto end;
	}

	data = mcp2515_can_id_build(id_mask, extended_id);
	res = mcp2515_register_write(pi_mcp2515, (uint8_t *)&data, 4, filter_mask_reg);

end:
	return (res);
}
/** @} */