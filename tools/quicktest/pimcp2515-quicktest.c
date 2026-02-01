/* Copyright 2026 Roos Catling-Tate
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

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pi_MCP2515.h"
#include "pi_MCP2515_defs.h"

#include "pimcp2515-quicktest.h"

int
main(void)
{
	pi_mcp2515_t *pi_mcp2515;
	pi_mcp2515_can_frame_t frame;
	int res, i;
	uint8_t reg_tmp = 0;
	bool tmp;

	stdio_init_all();

	/* Hardcoded stuff because this is just a dev tool. Maybe improve this later, maybe not. */
	if ((res = mcp2515_init(&pi_mcp2515, 0, 19, 16, 18, 17, 10000000, 8))) {
		printf("mcp2515_init failed: %d\n", res);
		return (res);
	} else
		printf("mcp2515_init succeeded\n");
	if ((res = mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_CONFIG))) {
		printf("mcp2515_reqop failed: %d\n", res);
		return (res);
	} else
		printf("mcp2515_reqop CONFIG succeeded\n");
	if ((res = mcp2515_bitrate_simplified(pi_mcp2515, 500))) {
		printf("mcp2515_bitrate_simplified failed: %d\n", res);
		return (res);
	} else
		printf("mcp2515_bitrate_simplified succeeded\n");
	if ((res = mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_LOOPBACK))) {
		printf("mcp2515_reqop failed: %d\n", res);
		return (res);
	}

	printf("CNF1: 0x%02x\nCNF2: 0x%02x\nCNF3: 0x%02x\n\n\n", mcp2515_cnf_get(pi_mcp2515, 1),
	    mcp2515_cnf_get(pi_mcp2515, 2), mcp2515_cnf_get(pi_mcp2515, 3));

	tmp = mcp2515_reqop_get(pi_mcp2515);
	printf("Checking reqop after mode changes. Excpected %02x, Got %02x\n", PI_MCP2515_REQOP_LOOPBACK, tmp);
	if (tmp != PI_MCP2515_REQOP_LOOPBACK) {
		printf("setting reqop LOOPBACK does not appear to have worked!");
		return (-1);
	}

	tmp = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg recieved before sending? %s\n", tmp ? "yes" : "no");
	if (tmp) {
		printf("CAN message received, but not expected!\n");
		return (-1);
	}

	memset(&frame, 0, sizeof(frame));
	frame.id = 0x0420;
	frame.dlc = sizeof(frame.payload);
	memset(frame.payload, 0x69, sizeof(frame.payload));
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	tmp = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg recieved after sending? %s\n", tmp ? "yes" : "no");
	if (tmp) {
		printf("CAN message not received, but expected!\n");
		return (-1);
	}

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  %08x\n  dlc: %02x\n  CAN data:\n   ");
	if (frame.dlc > CAN_FRAME_PAYLOAD_MAX) {
		printf("CAN msg received with invalid DLC length!\n");
		return (-1);
	}
	for (i = 0; i < frame.dlc; i++)
		printf(" %02x", frame.payload[i]);
	printf("\n");

	tmp = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg recieved after already reading? %s\n", tmp ? "yes" : "no");
	if (tmp) {
		printf("CAN message received, but not expected!\n");
		return (-1);
	}

	printf("status details:\n  status reg: %02x\n  eflag:      %02x\n  tx err:     %02x\n  rx err:     %02x\n",
	    mcp2515_status(pi_mcp2515), mcp2515_error_flags(pi_mcp2515), mcp2515_error_tx_count(pi_mcp2515),
	    mcp2515_error_rx_count(pi_mcp2515));

	/* TODO there are more functions that should be tested. */

	return (0);
}