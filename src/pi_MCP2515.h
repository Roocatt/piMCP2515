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

#ifndef PI_MCP2515_H
#define PI_MCP2515_H

#ifdef USE_PICO_LIB

#include "hardware/spi.h"

#endif /* USE_PICO_LIB */

#include <stdbool.h>
#include <stdint.h>

/* Register Definitions */
#define PI_MCP2515_RGSTR_CANCTRL 0x0F
#define PI_MCP2515_RGSTR_CANINTF 0x2C
#define PI_MCP2515_RGSTR_ECRX 0x1D
#define PI_MCP2515_RGSTR_ECTX 0x1C
#define PI_MCP2515_RGSTR_CNF1 0x2A
#define PI_MCP2515_RGSTR_CNF2 0x29
#define PI_MCP2515_RGSTR_CNF3 0x28
#define PI_MCP2515_RGSTR_RX0CTRL 0x60
#define PI_MCP2515_RGSTR_RX1CTRL 0x70

/* Instruction definitions */
#define PI_MCP2515_INSTR_WRITE 0x02
#define PI_MCP2515_INSTR_READ 0x03
#define PI_MCP2515_INSTR_BITMOD 0x05
#define PI_MCP2515_INSTR_READ_STATUS 0xA0
#define PI_MCP2515_INSTR_RESET 0xC0

/* REQOP definitions */
#define PI_MCP2515_REQOP_MASK 0xE0
#define PI_MCP2515_REQOP_NORMAL 0x00
#define PI_MCP2515_REQOP_SLEEP 0x20
#define PI_MCP2515_REQOP_LOOPBACK 0x40
#define PI_MCP2515_REQOP_LISTENONLY 0x60
#define PI_MCP2515_REQOP_CONFIG 0x80
#define PI_MCP2515_REQOP_POWERUP 0xE0

/* CTRL Definitions */
#define PI_MCP2515_CTRL_RTR 0x08
#define PI_MCP2515_CTRL_TXREQ 0x08
#define PI_MCP2515_CTRL_TXERR 0x10
#define PI_MCP2515_CTRL_MLOA 0x20
#define PI_MCP2515_CTRL_ABTF 0x40

/* Flag definitions */
#define PI_MCP2515_FLAG_RTR 0x40000000UL
#define PI_MCP2515_FLAG_EFF 0x80000000UL

/* RX/TX Status */
#define PI_MCP2515_STATUS_RX0BF 0x01
#define PI_MCP2515_STATUS_RX1BF 0x02

/* ID Mask Definitions */
#define PI_MCP2515_ID_MASK_SFF 0x000007FFUL
#define PI_MCP2515_ID_MASK_EFF 0x1FFFFFFFUL

#define PI_MCP2515_GPIO_PIN_MAP_LEN 40

typedef struct {
	uint32_t id;
	uint8_t dlc;
	uint8_t payload[8];
} pi_mcp2515_can_frame_t;

typedef struct {
	uint8_t spi_channel;
	uint8_t cs_pin;
	uint8_t tx_pin;
	uint8_t rx_pin;
	uint8_t sck_pin;
	uint32_t spi_clock;
#ifdef USE_PICO_LIB
	spi_inst_t *gpio_spi_inst;
#elifdef USE_SPIDEV
	int gpio_spidev_fd;
	int gpio_gpio_fd;
	uint8_t gpio_spi_mode;
	uint8_t gpio_spi_bits_per_word;
	uint16_t gpio_spi_delay_usec;
	int gpio_pin_fd_map[PI_MCP2515_GPIO_PIN_MAP_LEN]; /* TODO Info on GPIO pin/line mapping seems unclear. Verify size, etc. later. */
#endif
} pi_mcp2515_t;

int	mcp2515_can_message_send(pi_mcp2515_t *, const pi_mcp2515_can_frame_t *);
int	mcp2515_can_message_read(pi_mcp2515_t *, pi_mcp2515_can_frame_t *);
uint8_t	mcp2515_status(pi_mcp2515_t *);
int	mcp2515_register_read(pi_mcp2515_t *, uint8_t[], uint8_t, uint8_t);
int	mcp2515_register_write(pi_mcp2515_t *, uint8_t[], uint8_t, uint8_t);
int	mcp2515_register_bitmod(pi_mcp2515_t *, uint8_t, uint8_t, uint8_t);
int	mcp2515_reqop(pi_mcp2515_t *, uint8_t);
int	mcp2515_bitrate_default_16mhz_1000kbps(pi_mcp2515_t *);
int	mcp2515_bitrate_default_8mhz_500kbps(pi_mcp2515_t *);
int	mcp2515_bitrate_simplified(pi_mcp2515_t *, uint16_t, uint8_t);
int	mcp2515_bitrate_full_optional(pi_mcp2515_t *, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    bool, bool, bool, bool);
int	mcp2515_reset(pi_mcp2515_t *);
uint8_t	mcp2515_error_tx_count(pi_mcp2515_t *);
uint8_t	mcp2515_error_rx_count(pi_mcp2515_t *);
void	mcp2515_free(const pi_mcp2515_t *);
int	mcp2515_init(pi_mcp2515_t *, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);

#endif /* PI_MCP2515_H */
