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

#include "pimcp2515-pico-sh.h"

int
main(void)
{
	pi_mcp2515_t *pi_mcp2515;
	pi_mcp2515_can_frame_t frame;
	int res;
	char command[MAX_COMMAND_LENGTH];

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
	/* Default to loopback for testing */
	if ((res = mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_LOOPBACK))) {
		printf("mcp2515_reqop failed: %d\n", res);
		return (res);
	} else
		printf("mcp2515_reqop LOOPBACK succeeded\n");

	printf("CNF1: %02x\nCNF2: %02x\nCNF3: %02x\n\n\n", mcp2515_cnf_get(pi_mcp2515, 1),
	    mcp2515_cnf_get(pi_mcp2515, 2), mcp2515_cnf_get(pi_mcp2515, 3));

	/* Not worried about this being the best quality code, it is just to test the library. */
	for (;;) {
		printf(PICO_SH_PROMPT);
		if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL)
			continue; /* Since we're running on Pico, don't exit on EOF or error, just keep trying again. */

		command[strcspn(command, "\n")] = '\0';
		if (strnlen(command, MAX_COMMAND_LENGTH) == 0) {
			printf("\n");
			continue;
		}

		printf("command: \"%s\"\n", command);

		if (strncmp(command, CMD_REQOP, CMD_REQOP_LEN)) {
			if (strncmp(command + CMD_REQOP_LEN, CMD_REQOP_LOOPBACK, CMD_REQOP_LOOPBACK_LEN)) {
				PRINT_RES(mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_LOOPBACK));
			} else if (strncmp(command + CMD_REQOP_LEN, CMD_REQOP_NORMAL, CMD_REQOP_NORMAL_LEN)) {
				PRINT_RES(mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_NORMAL));
			} else if (strncmp(command + CMD_REQOP_LEN, CMD_REQOP_CONFIG, CMD_REQOP_CONFIG_LEN)) {
				PRINT_RES(mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_CONFIG));
			} else if (strncmp(command + CMD_REQOP_LEN, CMD_REQOP_SLEEP, CMD_REQOP_SLEEP_LEN)) {
				PRINT_RES(mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_SLEEP));
			} else if (strncmp(command + CMD_REQOP_LEN, CMD_REQOP_GET, CMD_REQOP_GET_LEN)) {
				printf("reqop: %02x\n", mcp2515_reqop_get(pi_mcp2515));
			} else {
				BAD_CMD_PRINT(command);
			}
		} else if (strncmp(command, CMD_CFG, CMD_CFG_LEN)) {
			if (strncmp(command + CMD_CFG_LEN, CMD_CFG_DUMP, CMD_CFG_DUMP_LEN)) {
				printf("CNF1: %02x, CNF2: %02x, CNF3: %02x\n", mcp2515_cnf_get(pi_mcp2515, 1),
				    mcp2515_cnf_get(pi_mcp2515, 2), mcp2515_cnf_get(pi_mcp2515, 3));
			} else
				BAD_CMD_PRINT(command);
		} else if (strncmp(command, CMD_CAN, CMD_CAN_LEN)) {
			if (strncmp(command + CMD_CAN_LEN, CMD_CAN_RECEIVED, CMD_CAN_RECEIVED_LEN)) {
				printf("CAN msg recieved: %s\n", mcp2515_can_message_received(pi_mcp2515) ? "true" : "false");
			} else if (strncmp(command + CMD_CAN_LEN, CMD_CAN_SEND, CMD_CAN_SEND_LEN)) {
				memset(&frame, 0, sizeof(frame));
				frame.id = 0x0420;
				frame.dlc = sizeof(frame.payload);
				memset(frame.payload, 0x69, sizeof(frame.payload));
				PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));
			} else if (strncmp(command + CMD_CAN_LEN, CMD_CAN_READ, CMD_CAN_READ_LEN)) {
				memset(&frame, 0, sizeof(frame));
				PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
				printf("got frame:\n  id:  %02x\n  dlc: %02x\n", frame.id);
			} else
				BAD_CMD_PRINT(command);
		} else if (strncmp(command, CMD_STATUS, CMD_STATUS_LEN)) {
			printf("status details:\n  status reg: %02x\n  eflag:      %02x\n  tx err:     %02x\n  rx err:     %02x\n",
			    mcp2515_status(pi_mcp2515), mcp2515_error_flags(pi_mcp2515), mcp2515_error_tx_count(pi_mcp2515),
			    mcp2515_error_rx_count(pi_mcp2515));
		} else if (strncmp(command, CMD_RESET, CMD_RESET_LEN)) {
			PRINT_RES(mcp2515_reset(pi_mcp2515));
		} else {
			BAD_CMD_PRINT(command);
		}
	}

	return (0);
}