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

#include <pi_MCP2515.h>

#include "internal.h"

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
 * @param filter the filter to configure.
 * @param id
 * @param extended_id
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter(pi_mcp2515_t *pi_mcp2515, const mcp2515_rxf_t filter, const uint32_t id, const bool extended_id)
{
	int res;
	uint32_t data;
	uint8_t filter_reg;

	switch (filter) {
	case PI_MCP2515_RXF0:
		filter_reg = PI_MCP2515_RGSTR_RXF0SIDH;
		break;
	case PI_MCP2515_RXF1:
		filter_reg = PI_MCP2515_RGSTR_RXF1SIDH;
		break;
	case PI_MCP2515_RXF2:
		filter_reg = PI_MCP2515_RGSTR_RXF2SIDH;
		break;
	case PI_MCP2515_RXF3:
		filter_reg = PI_MCP2515_RGSTR_RXF3SIDH;
		break;
	case PI_MCP2515_RXF4:
		filter_reg = PI_MCP2515_RGSTR_RXF4SIDH;
		break;
	case PI_MCP2515_RXF5:
		filter_reg = PI_MCP2515_RGSTR_RXF5SIDH;
		break;
	default:
		res = -1;
		goto end;
	}

	data = mcp2515_can_id_build(id, extended_id);
	res = mcp2515_register_write(pi_mcp2515, (uint8_t *)&data, 4, filter_reg);

end:
	return (res);
}

/**
 * @brief Set the filter mask. The MCP2515 must be in config mode to use this (see mcp2515_reqop/mcp2515_reqop_get).
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param filter_mask the filter mask register to use.
 * @param id_mask the ID mask to use.
 * @param extended_id if the ID is extended.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_filter_mask(pi_mcp2515_t *pi_mcp2515, const mcp2515_rxm_t filter_mask, const uint32_t id_mask, const bool extended_id)
{
	int res;
	uint32_t data;
	uint8_t filter_mask_reg;

	switch (filter_mask) {
	case PI_MCP2515_RXM0:
		filter_mask_reg = PI_MCP2515_RGSTR_RXM0SIDH;
		break;
	case PI_MCP2515_RXM1:
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

int
mcp2515_filter_enable(pi_mcp2515_t *pi_mcp2515, const bool enable)
{
	int res;

	res = mcp2515_filter_enable_rxb(pi_mcp2515, 0, enable);
	if (res != 0)
		goto end;
	res = mcp2515_filter_enable_rxb(pi_mcp2515, 1, enable);

end:
	return (res);
}

int
mcp2515_filter_enable_rxb(pi_mcp2515_t *pi_mcp2515, const mcp2515_rxb_t index, const bool enable)
{
	int res = -1;
	uint8_t reg;

	switch (index) {
	case PI_MCP2515_RXB0:
		reg = PI_MCP2515_RGSTR_RXB0CTRL;
		break;
	case PI_MCP2515_RXB1:
		reg = PI_MCP2515_RGSTR_RXB1CTRL;
		break;
	default:
		goto end;
	}

	res = mcp2515_register_bitmod(pi_mcp2515, enable ? 0x00 : 0xFF, PI_MCP2515_RXBCTRL_STDEXT_MASK, reg);

end:
	return (res);
}

/** @} */