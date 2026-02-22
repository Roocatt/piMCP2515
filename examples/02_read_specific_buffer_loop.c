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

int	print_from_buffer(pi_mcp2515_t *, mcp2515_rxb_t);

int
main(void)
{
	pi_mcp2515_t *mcp2515;
	int res;
	bool rx1_msg, rx2_msg;

	/* Init/setup. See 01_init_and_read_loop.c for comments outlining each step here. */
	mcp2515_init(&mcp2515, 0, 19, 16, 18, 17, 10000000, 8);
	mcp2515_reset(mcp2515);
	mcp2515_bitrate_simplified(mcp2515, 500);
	mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_NORMAL);

	for (;;) {
		/* Check both RX buffers individually. */
		rx1_msg = mcp2515_can_message_received_rxb(mcp2515, PI_MCP2515_RXB0);
		rx2_msg = mcp2515_can_message_received_rxb(mcp2515, PI_MCP2515_RXB1);

		/* If neither buffer has a message, then wait and try again. */
		if (!rx1_msg && !rx2_msg) {
			mcp2515_micro_sleep(5000);
			continue;
		}

		/* Handle RXB0 and/or RXB1 based on if either has a message, before the loop continues to check. Unlike
		 * 01_init_and_read_loop.c, it is not possible for a message to get stuck in RXB1.
		 *
		 * One of the annoyances with this over the example in 01_init_and_read_loop.c, is that it necessitates
		 * at least a tiny bit of repetition as you can see here.
		 */
		if (rx1_msg) {
			res = print_from_buffer(mcp2515, PI_MCP2515_RXB0);
			if (res != 0) {
				fprintf(stderr, "mcp2515_can_message_read() returned %d\n", res);
				break;
			}
		}
		if (rx2_msg) {
			res = print_from_buffer(mcp2515, PI_MCP2515_RXB1);
			if (res != 0) {
				fprintf(stderr, "mcp2515_can_message_read() returned %d\n", res);
				break;
			}
		}
	}

	return (0);
}

int
print_from_buffer(pi_mcp2515_t *mcp2515, const mcp2515_rxb_t rxb)
{
	pi_mcp2515_can_frame_t frame = {0};
	int res;
	uint8_t i;

	/* Read the CAN bus message. Just like with `mcp2515_can_message_received_rxb` in `main` in this example, the
	 * specific RX buffer to read from is passed.
	 */
	res = mcp2515_can_message_read_rxb(mcp2515, rxb, &frame);
	if (res != 0)
		goto end;

	/* By splitting off the message handling (in the case of this example, just printing) to its own function, it is
	 * possible to avoid some of the repetition mention in a comment in main.
	 */
	printf("CAN msg retrieved:\n  id:  0x%08lx\n  dlc: 0x%02x\n  CAN data:\n   ", frame.id, frame.dlc);
	for (i = 0; i < frame.dlc; i++)
		printf(" 0x%02x", frame.payload[i]);
	printf("\n\n");

end:
	return (res);
}
