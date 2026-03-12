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

#include <pi_MCP2515.h>

#include "pimcp2515-hello-world.h"

/* Helpful tool for testing CAN communication between multiple devices. It just
 * repeatedly transmits a message and prints whatever message it receives.
 */

int
main()
{
	pi_mcp2515_t *mcp2515;
	pi_mcp2515_can_frame_t tx_frame = {0}, rx_frame;
	int res;
	uint8_t i;

#ifdef USE_PICO_LIB
	stdio_init_all();
#endif

	printf("Starting PiMCP2515 Hello-World...\n");
	mcp2515_init(&mcp2515, 0, 19, 16, 18, 17, 10000000, 8);
	mcp2515_debug_enable(mcp2515, NULL);
	mcp2515_reset(mcp2515);
	mcp2515_bitrate_full_optional(mcp2515, 500, 2, 0, 2, 2, 3, false, false, false, true);
	mcp2515_filter_enable(mcp2515, false);
	mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_NORMAL);

	printf("Init complete, sending and listening for messages...\n");

	tx_frame.id = 0x0bad;
	tx_frame.payload[0] = 'H';
	tx_frame.payload[1] = 'e';
	tx_frame.payload[2] = 'l';
	tx_frame.payload[3] = 'l';
	tx_frame.payload[4] = 'o';
	tx_frame.payload[5] = '!';
	tx_frame.dlc = 6;

	for (;;) {
		memset(&rx_frame, 0, sizeof(rx_frame));
		res = mcp2515_can_message_send(mcp2515, &tx_frame);
		if (res)
			printf("mcp2515_can_send() failed: %d\n", res);
		else
			printf("mcp2515_can_send()\n");

		if (mcp2515_can_message_received(mcp2515)) {
			res = mcp2515_can_message_read(mcp2515, &rx_frame);
			if (res)
				printf("mcp2515_can_read() failed: %d\n", res);
			else {
				printf("recv id: 0x%04lx\n", rx_frame.id);
				for (i = 0; i < rx_frame.dlc; i++)
					printf("0x%02x ", rx_frame.payload[i]);
				printf("\n");
			}
		}

		mcp2515_micro_sleep(1000000);
	}

	return (0);
}