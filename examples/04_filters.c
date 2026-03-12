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

/* TODO This would be a better example/demo if using loopback mode and sending messages to show
 * filtering in action. When this is complete, it should also be added to `quicktest`.
 */

void	print_messages(pi_mcp2515_t *);

int
main(void)
{
	pi_mcp2515_t *mcp2515;

	/* Init/setup. See 01_init_and_read_loop.c for comments outlining each step here. */
	mcp2515_init(&mcp2515, 0, 19, 16, 18, 17, 10000000, 8);
	mcp2515_reset(mcp2515);
	mcp2515_bitrate_simplified(mcp2515, 500);
	mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_NORMAL);

	/* Turn off (or on) filters with `mcp2515_filter_enable`. The boolean value indicates if
	 * the function call is to enable or disable the filters.
	 */
	mcp2515_filter_enable(mcp2515, false);

	/* Read a few messages to show operating without a filter. */
	print_messages(mcp2515);

	/* Enable the filters again to show them in action. */
	mcp2515_filter_enable(mcp2515, true);

	/* Set the filter masks.
	 *
	 * See this table from the MCP2515 documentation. Basically, the mask determines if the
	 * bit is handled by the individual filters.
	 * +------------+----------------------------------------------------------------+
	 * | Mask Bit n | Filter Bit n | Message Identifier Bit | Accept or Reject Bit n |
	 * |          0 |            x |                      x |                 Accept |
	 * |          1 |            0 |                      0 |                 Accept |
	 * |          1 |            0 |                      1 |                 Reject |
	 * |          1 |            1 |                      0 |                 Reject |
	 * |          1 |            1 |                      1 |                 Accept |
	 * +------------+----------------------------------------------------------------+
	 *
	 * It should also be noted that the RXM0 mask applies to RXB0, and RXM1 to RXB1.
	 * Additionally, RXF0 and RXF1 apply only to RXB0, while RXF2, RXF3, RXF4, and
	 * RXF5 apply to RXB1. Per the table above, the two filter masks only apply to
	 * their respective buffer.
	 *
	 * This makes it possible to direct certain IDs to a specific buffer if desired. RXB0
	 * is prioritized so if it matches the filters of RXB0 and RXB1 then RXB0 will receive
	 * the message. That is unless RXB0 is full, in which case it will go to RXB1. Should
	 * RXB1 also be full, then an overflow interrupt will be generated.
	 *
	 * Importantly, the filter applies to the first two data bytes for standard ID
	 * messages. This is important to consider as for many cases that will not be
	 * desirable. The boolean argument to `mcp2515_filter_mask` indicates if it is an
	 * extended ID being provided. If this is false, then the ID will be assembled
	 * accordingly and not apply to the data bytes.
	 *
	 * If you do want to use data filtering then you will need to pass true so it is treated
	 * as an extended ID. In that case, you will use the most significant 16 bits for the ID
	 * passed as the id_mask argument and the 16 least significant bits for data filtering.
	 * The notable asterix here is that when the ID is assembled, the full 16 most significant
	 * bits are not used, only those covered by the mask PI_MCP2515_CAN_ID_EFF_MASK as the
	 * actual ID is only 11 bits. The easiest way to assemble this would be something like:
	 * `(your_id << 16) | (data_bytes[0] << 8) | data_bytes[1]`
	 *
	 * PI_MCP2515_CAN_ID_SFF_MASK is used here as it applies the filter to any bits that
	 * make up a standard ID.
	 */
	mcp2515_filter_mask(mcp2515, PI_MCP2515_RXM0, PI_MCP2515_CAN_ID_SFF_MASK, false);
	mcp2515_filter_mask(mcp2515, PI_MCP2515_RXM1, PI_MCP2515_CAN_ID_SFF_MASK, false);

	/* Set up the RXF0 filter. With both masks set as above, at this point only a zero ID
	 * would go to RXB1, and those matching this filter will be allowed for RXB0.
	 */
	mcp2515_filter(mcp2515, PI_MCP2515_RXF0, 0x00000420, false);

	/* This adds a second filter with different criteria to RXB0. This now means any messages
	 * matching either filter are accepted into RXB0, whereas RXB1 will still only receive
	 * messages with an ID of zero.
	 */
	mcp2515_filter(mcp2515, PI_MCP2515_RXF1, 0x00000069, false);

	/* Read a few messages to show operating with a filter now configured. */
	print_messages(mcp2515);

	/* Now we set the mask for RXB1 to all zeroes, which will keep the same filter criteria for
	 * RXB0, but RXB1 will now receive all messages.
	 */
	mcp2515_filter_mask(mcp2515, PI_MCP2515_RXM1, 0x00000000, false);

	/* Read a few messages to show operating with the new value for RXM1. */
	print_messages(mcp2515);

	/* Cleanup after. */
	mcp2515_free(mcp2515);

	return (0);
}

/* Receive and print 5 messages, or just wait the timeout. Not important really, this is just
 * to show the filters. To see reading in detail refer to 01_init_and_read_loop and
 * 02_read_specific_buffer_loop.
 */
void
print_messages(pi_mcp2515_t *mcp2515)
{
	pi_mcp2515_can_frame_t frame;
	uint32_t i;

	for (i = 0; i < 5; i++) {
		if (!mcp2515_can_message_received(mcp2515)) {
			mcp2515_micro_sleep(5000);
			continue;
		}
		memset(&frame, 0, sizeof(frame));
	}
}