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

#include "../../include/pi_MCP2515.h"
#include "../../include/pi_MCP2515_defs.h"

#include "pimcp2515-quicktest.h"


/* This is not 100% comprehensive for testing functionality, but it does call most functions. Ideally this should be
 * expanded to really cover everything, but this will do for now.
 */
int
main()
{
	pi_mcp2515_t *pi_mcp2515;
	pi_mcp2515_can_frame_t frame;
	int i;
	uint8_t reg_tmp = 0, checkpoint = 1;
	bool tmp_bool;

	stdio_init_all();

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

	checkpoint = 2;

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

	checkpoint = 3;

	printf("CNF1: 0x%02x\nCNF2: 0x%02x\nCNF3: 0x%02x\n", mcp2515_cnf_get(pi_mcp2515, 1),
	    mcp2515_cnf_get(pi_mcp2515, 2), mcp2515_cnf_get(pi_mcp2515, 3));

	printf("\n");

	PRINT_RES(mcp2515_filter_mask(pi_mcp2515, PI_MCP2515_RGSTR_RXM0SIDH, 0, true));
	PRINT_RES(mcp2515_filter_mask(pi_mcp2515, PI_MCP2515_RGSTR_RXM1SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF0SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF1SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF2SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF3SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF4SIDH, 0, true));
	PRINT_RES(mcp2515_filter(pi_mcp2515, PI_MCP2515_RGSTR_RXF5SIDH, 0, true));

	printf("\n");

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received before sending? %s\n", tmp_bool ? "yes" : "no");
	if (tmp_bool) {
		printf("CAN message received, but not expected!\n");
		goto end;
	}

	checkpoint = 4;

	printf("\n");

	memset(&frame, 0, sizeof(frame));
	frame.id = 0x0420;
	frame.dlc = sizeof(frame.payload);
	memset(frame.payload, 0x69, sizeof(frame.payload));
	PRINT_RES(mcp2515_can_message_send(pi_mcp2515, &frame));

	printf("\n");

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received after sending? %s\n\n", tmp_bool ? "yes" : "no");
	if (!tmp_bool) {
		printf("CAN message not received, but expected!\n");
		goto end;
	}

	checkpoint = 5;

	printf("\n");

	memset(&frame, 0, sizeof(frame));
	PRINT_RES(mcp2515_can_message_read(pi_mcp2515, &frame));
	printf("CAN msg retrieved:\n  id:  %08lx\n  dlc: 0x%02x\n  CAN data:\n   ", frame.id, frame.dlc);
	if (frame.dlc > CAN_FRAME_PAYLOAD_MAX) {
		printf("CAN msg received with invalid DLC length!\n");
		goto end;
	}
	for (i = 0; i < frame.dlc; i++)
		printf(" 0x%02x", frame.payload[i]);
	printf("\n\n");

	checkpoint = 6;

	tmp_bool = mcp2515_can_message_received(pi_mcp2515);
	printf("Is CAN msg received after already reading? %s\n\n", tmp_bool ? "yes" : "no");
	if (tmp_bool) {
		printf("CAN message received, but not expected!\n");
		goto end;
	}

	printf("\n");

	reg_tmp = mcp2515_interrupts_get(pi_mcp2515);
	printf("interrupt flags: %02x\n", reg_tmp);
	reg_tmp = mcp2515_interrupts_mask(pi_mcp2515);
	printf("interrupt mask: %02x\n", reg_tmp);

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
			for (i = 0; i < checkpoint; i++) {
				gpio_put(PICO_DEFAULT_LED_PIN, true);
				sleep_ms(200);
				gpio_put(PICO_DEFAULT_LED_PIN, false);
				sleep_ms(200);
			}
			sleep_ms(1000);
		}
	}

	return (0);
}