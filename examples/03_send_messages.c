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
	pi_mcp2515_can_frame_t frame = {0};
	int res;

	/* Init/setup. See 01_init_and_read_loop.c for comments outlining each step here. */
	mcp2515_init(&mcp2515, 0, 19, 16, 18, 17, 10000000, 8);
	mcp2515_reset(mcp2515);
	mcp2515_bitrate_simplified(mcp2515, 500);
	mcp2515_reqop(mcp2515, PI_MCP2515_REQOP_NORMAL);

	/* Prepare the frame to send. This example is using an extended ID, which allows for the ID to ocupy any of the
	 * bits defined by the mask `0x1FFFFFFF`. If not using an extended ID, then only the bits defined by the mask
	 * `0x000007FF` can be used.
	 *
	 * piMCP2515 will exclude bits from the ID if the `id` field exceeds what is expected for a standard ID if
	 * `extended_id` is false. Conversely, piMCP2515 include zero bits from the mask `0xFFFFF800 if the `id` field
	 * is a standard ID and `extended_id` is true. This is of course to say, make sure that `extended_id` is
	 * correctly true or false in accordance with the ID being passed.
	 */
	frame.id = 0x00420420;
	frame.extended_id = true;

	/* DLC is data length code, and it represents the length of the data payload (or length of the data requested
	 * for RTR, but more on that later...). This can be anywhere from zero to eight. This is of course also the
	 * range of data length that can be put into the payload.
	 */
	frame.dlc = sizeof(frame.payload);
	memset(frame.payload, 0x69, sizeof(frame.payload));

	/* Send the payload... */
	res = mcp2515_can_message_send(mcp2515, &frame);
	if (res != 0) {
		fprintf(stderr, "mcp2515_can_message_send failed\n");
		goto end;
	}

	memset(&frame, 0x00, sizeof(frame));

	/* Send another message. This time an RTR message with a standard ID. RTR messages can use standard or extended
	 * IDs, but the preceding send showed sending with an extended ID so this part of the example will show a
	 * standard ID for completeness.
	 */
	frame.id = 0x00000420;
	frame.extended_id = false;
	frame.rtr = true;

	/* RTR is a remote transmission request. It is a request for data from another CAN node, though it is of note
	 * that nodes normally just automatically keep sending data without it being requested.
	 *
	 * RTR frames don't actually contain a payload, however DLC is still set as it instead indicates the length of
	 * data requested (though this is usually just the maximum size).
	 */
	frame.dlc = sizeof(frame.payload);

	/* Send the RTR message... */
	res = mcp2515_can_message_send(mcp2515, &frame);
	if (res != 0)
		fprintf(stderr, "mcp2515_can_message_send failed\n");

end:
	return (0);
}
