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

#include <string.h>

#include <pi_MCP2515.h>
#include <stdio.h>

int
main(void)
{
	pi_mcp2515_t *mcp2515;
	pi_mcp2515_can_frame_t frame;
	int res;
	uint8_t i;

	/* Init the mcp2515 handle.
	 * Use SPI channel 0.
	 * 19, 16, 18, and 17 are defaults for the pico.
	 * 10000000 SPI clock.
	 * 8Mhz oscillator.
	 */
	mcp2515_init(&mcp2515, 0, 19, 16, 18, 17, 10000000, 8);

	/* It is recommended to call reset as part of the init process for the MCP2515. This will always return 0 on the
	 * Pi Pico. If not using the Pico or writing code that might be compiled for both the Pico and other platforms
	 * then you should check the return and handle a non-zero value.
	 *
	 * Reset automatically sleeps to await the MCP2515 being ready again.
	 *
	 * It is important to note, however, that calling `mcp2515_reset` will result in the device being put in config
	 * mode, meaning you will not have to change the mode if you call reset as is recommended. To otherwise switch
	 * to config mode it would be `mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_CONFIG)`.
	 */
	mcp2515_reset(mcp2515);

	/* In either the full or simplified variation here, the MCP2515 must be in config mode. However, per the above
	 * comment, `mcp2515_reset` will handle that.
	 */
#ifdef FULL_OPTIONAL_CONFIG
	/* Pass the full configuration options to set up the MCP2515. */
	mcp2515_bitrate_full_optional(mcp2515, 500, 2, 0, 2, 2, 3, false, false, false, true);
#else
	/* A simplified config function. For most cases, this is all you need. In fact, other libraries for the MCP2515
	 * generally use a precomputed table for configuration and don't provide a full configuration at all. Unless one
	 * of the specific full configuration options are needed, this is the preferred approach.
	 */
	mcp2515_bitrate_simplified(mcp2515, 500);
#endif

	/* Change to normal mode after configuration is done. */
	mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_NORMAL);

	for (;;) {
		if (!mcp2515_can_message_received(mcp2515)) {
			/* piMCP2515 provides a sleep function, though there is nothing special about it. It exists
			 * largely for convenience internally such that it can be conditionally defined based of the
			 * platform it is compiled for so that Pico, Linux, and BSD all just use `mcp2515_micro_sleep`.
			 */
			mcp2515_micro_sleep(5000);
			continue;
		}

		memset(&frame, 0, sizeof(frame));

		/* Read the CAN bus message. */
		res = mcp2515_can_message_read(mcp2515, &frame);

		if (res != 0) {
			fprintf(stderr, "mcp2515_can_message_read() returned %d\n", res);
			break;
		}

		/* The MCP2515 has two RX buffers, RXB0 and RXB1. In the case of `mcp2515_can_message_read` and with
		 * `mcp2515_can_message_received`, piMCP2515 favours reading RXB0 if there is a message in both buffers,
		 * even if RXB1 has had a message waiting to be read for longer. While it hasn't been observed, it is
		 * theoretically possible to have a scenario where there is enough CAN traffic that you can't get an
		 * iteration where RXB0 is empty to be able to read RXB1. It is possible to avoid this if you use the
		 * functions to check specific RX buffers, though for many use cases it may not be worth the added
		 * effort.
		 *
		 * See 02_read_specific_buffer_loop.c for an example handling individual buffers.
		 */

		printf("CAN msg retrieved:\n  id:  0x%08lx\n  dlc: 0x%02x\n  CAN data:\n   ", frame.id, frame.dlc);
		for (i = 0; i < frame.dlc; i++)
			printf(" 0x%02x", frame.payload[i]);
		printf("\n\n");
	}

	return (0);
}
