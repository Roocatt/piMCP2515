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

#include <pi_MCP2515.h>

#include "pimcp2515-quicktest.h"

static int checkpoint = 0;

static void	banner_print(char *);
static void
banner_print(char *str)
{
	uint8_t i;

	for (i = 0; i < 100; i++)
		printf("*");
	printf("\n");
	printf(str);
	printf("\n");
	for (i = 0; i < 100; i++)
		printf("*");
	printf("\n\n");
}

static void	set_checkpoint(int, char *);

static void
set_checkpoint(int c, char *title)
{
	char buf[128] = {0};

	sprintf(buf, "     Checkpoint #%d - %s", c, title);

	checkpoint = c;

	banner_print(buf);
}

/* This is not 100% comprehensive for testing functionality, but it does call most functions. Ideally this should be
 * expanded to really cover everything, but this will do for now.
 *
 * Currently this is only for the Pico, which means a number of functions (ex `mcp2515_register_bitmod`) will always
 * return `0`. Logging return code with the `PRINT_RES` macro is still done as it helps view progress through the run
 * and this will probably be ported to support testing other platforms at some point.
 */
int
main()
{
	pi_mcp2515_t *pi_mcp2515;
	pi_mcp2515_can_frame_t frame;
	int i;
	uint32_t id;
	uint8_t reg_tmp = 0;
	bool tmp_bool;

	stdio_init_all();

	banner_print("Starting Test Run...");

	printf("\n");
	set_checkpoint(1, "Init, Reset, and REQOP");

	/* Hardcoded stuff because this is just a dev tool. Maybe improve this later, maybe not. */
	/* This is for the Pico, so returns are always 0 */
	mcp2515_init(&pi_mcp2515, 0, 19, 16, 18, 17, 10000000, 8);
	mcp2515_debug_enable(pi_mcp2515, NULL);
	printf("mcp2515_init\n");
	PRINT_RES(mcp2515_reset(pi_mcp2515));
	sleep_us(mcp2515_osc_time(pi_mcp2515, 128) + 30);

	printf("\n");

	mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_CONFIG);
	printf("mcp2515_reqop CONFIG\n");

	reg_tmp = mcp2515_reqop_get(pi_mcp2515);
	printf("Checking reqop after mode changes. Expected 0x%02x, Got 0x%02x\n", PI_MCP2515_REQOP_CONFIG, reg_tmp);
	if (reg_tmp != PI_MCP2515_REQOP_CONFIG) {
		printf("setting reqop LOOPBACK does not appear to have worked!\n");
		goto end;
	}
	printf("\n");

	set_checkpoint(2, "Set bitrate and LOOPBACK mode");

	mcp2515_bitrate_simplified(pi_mcp2515, 500);
	printf("mcp2515_bitrate_simplified\n");

	printf("\n");

	mcp2515_reqop(pi_mcp2515, PI_MCP2515_REQOP_LOOPBACK);
	printf("mcp2515_reqop LOOPBACK\n");
	reg_tmp = mcp2515_reqop_get(pi_mcp2515);
	printf("Checking reqop after mode changes. Expected 0x%02x, Got 0x%02x\n", PI_MCP2515_REQOP_LOOPBACK, reg_tmp);
	if (reg_tmp != PI_MCP2515_REQOP_LOOPBACK) {
		printf("setting reqop LOOPBACK does not appear to have worked!\n");
		goto end;
	}
	printf("\n");
	printf("CNF1: 0x%02x\nCNF2: 0x%02x\nCNF3: 0x%02x\n", mcp2515_cnf_get(pi_mcp2515, 1),
	    mcp2515_cnf_get(pi_mcp2515, 2), mcp2515_cnf_get(pi_mcp2515, 3));

	set_checkpoint(3, "Disable filter and check for messages before begining");

	printf("\n");

	PRINT_RES(mcp2515_filter_enable(pi_mcp2515, false));

	printf("\n");

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received before sending? %s\n", tmp_bool ? "yes" : "no");
	if (tmp_bool) {
		printf("CAN message received, but not expected!\n");
		goto end;
	}

	set_checkpoint(4, "Send standard message");

	printf("\n");

	memset(&frame, 0, sizeof(frame));
	frame.id = 0x00000420;
	frame.dlc = sizeof(frame.payload);
	memset(frame.payload, 0x69, sizeof(frame.payload));
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	mcp2515_micro_sleep(3);

	printf("\n");

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received after sending? %s\n\n", tmp_bool ? "yes" : "no");
	if (!tmp_bool) {
		printf("CAN message not received, but expected!\n");
		goto end;
	}

	set_checkpoint(5, "Retreive message");

	printf("\n");

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  0x%08lx\n  dlc: 0x%02x\n  CAN data:\n   ", frame.id, frame.dlc);
	if (frame.dlc > PI_MCP2515_CAN_FRAME_PAYLOAD_MAX) {
		printf("CAN msg received with invalid DLC length!\n");
		goto end;
	}
	if (frame.id != 0x00000420 || frame.dlc != sizeof(frame.payload)) {
		printf("Frame or DLC incorrect in received message\n");
		goto end;
	}
	for (i = 0; i < frame.dlc; i++)
		printf(" 0x%02x", frame.payload[i]);
	printf("\n\n");

	set_checkpoint(6, "Check after reading message");

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received after already reading? %s\n\n", tmp_bool ? "yes" : "no");
	if (tmp_bool) {
		printf("CAN message received, but not expected!\n");
		goto end;
	}

	printf("\n");

	reg_tmp = mcp2515_interrupts_get(pi_mcp2515);
	printf("interrupt flags: 0x%02x\n", reg_tmp);
	reg_tmp = mcp2515_interrupts_mask(pi_mcp2515);
	printf("interrupt mask: 0x%02x\n", reg_tmp);

	printf("\n");

	/* Repeat for extended ID. */
	set_checkpoint(7, "Test send/receive EID message");
	frame.id = 0x0420420;
	frame.extended_id = true;
	frame.dlc = sizeof(frame.payload);
	memset(frame.payload, 0x69, sizeof(frame.payload));
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	printf("\n");
	mcp2515_status(pi_mcp2515);

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  0x%08lx\n  eid: %d\n  dlc: 0x%02x\n", frame.id, frame.extended_id, frame.dlc);
	if (frame.dlc > PI_MCP2515_CAN_FRAME_PAYLOAD_MAX) {
		printf("CAN msg received with invalid DLC length!\n");
		goto end;
	}
	if (frame.id != 0x0420420 || frame.dlc != 8 || !frame.extended_id) {
		printf("Frame or DLC incorrect in received message\n");
		goto end;
	}
	printf("\n");

	/* Repeat for RTR */
	set_checkpoint(8, "Test send/receive RTR message");
	frame.id = 0x00000420;
	frame.rtr = true;
	frame.dlc = sizeof(frame.payload);
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	printf("\n");
	mcp2515_status(pi_mcp2515);

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  0x%08lx\n  rtr: %d\n  dlc: 0x%02x\n", frame.id, frame.rtr, frame.dlc);
	if (frame.id != 0x00000420 || frame.dlc != 8 || !frame.rtr) {
		printf("Frame or DLC incorrect in received message\n");
		goto end;
	}
	printf("\n");

	/* Repeat for both EID and  RTR */
	set_checkpoint(9, "Test send/receive RTR and EID message");
	frame.id = 0x00420420;
	frame.extended_id = true;
	frame.rtr = true;
	frame.dlc = sizeof(frame.payload);
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	printf("\n");
	mcp2515_status(pi_mcp2515);

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  0x%08lx\n  eid: %d\n  rtr: %d\n  dlc: 0x%02x\n", frame.id, frame.extended_id,
	    frame.rtr, frame.dlc);
	if (frame.id != 0x0420420 || frame.dlc != 8 || !frame.rtr || !frame.rtr) {
		printf("Frame or DLC incorrect in received message\n");
		goto end;
	}
	printf("\n");

	checkpoint = 0;

end:
	printf("status details:\n  status reg: 0x%02x\n  eflag:      0x%02x\n  tx err:     0x%02x\n  rx err:     0x%02x\n",
	    mcp2515_status(pi_mcp2515), mcp2515_error_flags(pi_mcp2515), mcp2515_error_tx_count(pi_mcp2515),
	    mcp2515_error_rx_count(pi_mcp2515));

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	
	if (checkpoint == 0) {
		gpio_put(PICO_DEFAULT_LED_PIN, true);
	} else {
		for (;;) {
			for (i = 0; i < checkpoint / 5; i++) {
				gpio_put(PICO_DEFAULT_LED_PIN, true);
				sleep_ms(500);
				gpio_put(PICO_DEFAULT_LED_PIN, false);
				sleep_ms(250);
			}
			for (i = 0; i < checkpoint % 5; i++) {
				gpio_put(PICO_DEFAULT_LED_PIN, true);
				sleep_ms(250);
				gpio_put(PICO_DEFAULT_LED_PIN, false);
				sleep_ms(250);
			}
			sleep_ms(1000);
		}
	}

	return (0);
}