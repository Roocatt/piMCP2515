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

/* External Header.
 *
 * This is for including in projects that use the library. It must match up to the internal headers, which is currently
 * done manually. I don't really like this approach, but it will do the job for now.
 *
 * TODO First draft. Tidy and finish the external header stuff.
 */

#ifndef PIMCP2515_PI_MCP2515_H
#define PIMCP2515_PI_MCP2515_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define PI_MCP2515_CAN_FRAME_PAYLOAD_MAX 8

/**
 * @brief CAN bus frame data structure.
 */
typedef struct {
	uint32_t id;
	bool extended_id;
	bool rtr;
	uint8_t dlc;
	uint8_t payload[PI_MCP2515_CAN_FRAME_PAYLOAD_MAX];
} pi_mcp2515_can_frame_t;

/**
 * @defgroup piMCP2515_register_addresses Register Addresses
 * @brief These definitions hold the MCP2515 register addresses.
 * @{
 */
typedef enum {
	PI_MCP2515_RGSTR_RXF0SIDH = 0x00,
	PI_MCP2515_RGSTR_RXF1SIDH = 0x04,
	PI_MCP2515_RGSTR_RXF2SIDH = 0x08,
	PI_MCP2515_RGSTR_RXF3SIDH = 0x10,
	PI_MCP2515_RGSTR_RXF4SIDH = 0x14,
	PI_MCP2515_RGSTR_RXF5SIDH = 0x18,
	PI_MCP2515_RGSTR_RXM0SIDH = 0x20,
	PI_MCP2515_RGSTR_RXM1SIDH = 0x24,
	PI_MCP2515_RGSTR_CANSTAT = 0x0E,
	PI_MCP2515_RGSTR_CANCTRL = 0x0F,
	PI_MCP2515_RGSTR_CANINTE = 0x2B,
	PI_MCP2515_RGSTR_CANINTF = 0x2C,
	PI_MCP2515_RGSTR_EFLG = 0x2D,
	PI_MCP2515_RGSTR_ECRX = 0x1D,
	PI_MCP2515_RGSTR_ECTX = 0x1C,
	PI_MCP2515_RGSTR_CNF1 = 0x2A,
	PI_MCP2515_RGSTR_CNF2 = 0x29,
	PI_MCP2515_RGSTR_CNF3 = 0x28,
	PI_MCP2515_RGSTR_RXB0CTRL = 0x60,
	PI_MCP2515_RGSTR_RXB0SIDH = 0x61,
	PI_MCP2515_RGSTR_RXB0SIDL = 0x62,
	PI_MCP2515_RGSTR_RXB0DATA = 0x66,
	PI_MCP2515_RGSTR_RXB1CTRL = 0x70,
	PI_MCP2515_RGSTR_RXB1SIDH = 0x71,
	PI_MCP2515_RGSTR_RXB1SIDL = 0x72,
	PI_MCP2515_RGSTR_RXB1DATA = 0x76,
	PI_MCP2515_RGSTR_TXB0CTRL = 0x30,
	PI_MCP2515_RGSTR_TXB0SIDH = 0x31,
	PI_MCP2515_RGSTR_TXB0SIDL = 0x32,
	PI_MCP2515_RGSTR_TXB0EID8 = 0x33,
	PI_MCP2515_RGSTR_TXB0EID0 = 0x34,
	PI_MCP2515_RGSTR_TXB0DLC = 0x35,
	PI_MCP2515_RGSTR_TXB0DATA = 0x36,
	PI_MCP2515_RGSTR_TXB1CTRL = 0x40,
	PI_MCP2515_RGSTR_TXB1SIDH = 0x41,
	PI_MCP2515_RGSTR_TXB1SIDL = 0x42,
	PI_MCP2515_RGSTR_TXB1EID8 = 0x43,
	PI_MCP2515_RGSTR_TXB1EID0 = 0x44,
	PI_MCP2515_RGSTR_TXB1DLC = 0x45,
	PI_MCP2515_RGSTR_TXB1DATA = 0x46,
	PI_MCP2515_RGSTR_TXB2CTRL = 0x50,
	PI_MCP2515_RGSTR_TXB2SIDH = 0x51,
	PI_MCP2515_RGSTR_TXB2SIDL = 0x52,
	PI_MCP2515_RGSTR_TXB2EID8 = 0x53,
	PI_MCP2515_RGSTR_TXB2EID0 = 0x54,
	PI_MCP2515_RGSTR_TXB2DLC = 0x55,
	PI_MCP2515_RGSTR_TXB2DATA = 0x56,
} mcp2515_rgstr_t;
/** @} */

/**
 * @defgroup piMCP2515_instructions Instruction Codes
 * @brief These definitions hold the instruction codes.
 * @{
 */
#define PI_MCP2515_INSTR_WRITE 0x02 /**< @brief SPI interface write instruction. */
#define PI_MCP2515_INSTR_READ 0x03 /**< @brief SPI interface read instruction. */
#define PI_MCP2515_INSTR_BITMOD 0x05 /**< @brief SPI interface bit modify instruction. */
#define PI_MCP2515_INSTR_READ_STATUS 0xA0 /**< @brief SPI interface status instruction. */
#define PI_MCP2515_INSTR_RX_STATUS 0xB0 /**< @brief SPI interface RX status instruction. */
#define PI_MCP2515_INSTR_RESET 0xC0 /**< @brief SPI interface reset instruction. */

#define PI_MCP2515_INSTR_LOAD_TX0 0x40
#define PI_MCP2515_INSTR_LOAD_TX1 0x42
#define PI_MCP2515_INSTR_LOAD_TX2 0x44

#define PI_MCP2515_INSTR_READ_RX0 0x90
#define PI_MCP2515_INSTR_READ_RX1 0x94

/* These can be `|`'d together to apply the RTS instruction to multiple buffers. */
#define PI_MCP2515_INSTR_RTS_TX0 0x81
#define PI_MCP2515_INSTR_RTS_TX1 0x82
#define PI_MCP2515_INSTR_RTS_TX2 0x84
/** @} */

/**
 * @defgroup piMCP2515_reqops REQOP Codes
 * @brief These definitions hold the REQOP (operating mode) codes.
 * @{
 */
typedef enum {
	PI_MCP2515_REQOP_NORMAL = 0x00, /**< @brief Normal operating mode. */
	PI_MCP2515_REQOP_SLEEP = 0x20, /**< @brief Sleep operating mode. */
	PI_MCP2515_REQOP_LOOPBACK = 0x40, /**< @brief Loopback operating mode. */
	PI_MCP2515_REQOP_LISTENONLY = 0x60, /**< @brief Listenonly operating mode. */
	PI_MCP2515_REQOP_CONFIG = 0x80, /**< @brief Config operating mode. */
	PI_MCP2515_REQOP_POWERUP = 0xE0, /**< @brief Powerup operating mode. */
} mcp2515_reqop_t;
#define PI_MCP2515_REQOP_MASK 0xE0 /**< @brief Mask for REQOP values. */
/** @} */

/* CTRL Definitions */
#define PI_MCP2515_CTRL_RTR 0x08
#define PI_MCP2515_CTRL_TXREQ 0x08
#define PI_MCP2515_CTRL_TXERR 0x10
#define PI_MCP2515_CTRL_MLOA 0x20
#define PI_MCP2515_CTRL_ABTF 0x40
#define PI_MCP2515_RXBCTRL_STDEXT_MASK 0x60

/**
 * @defgroup piMCP2515_eflg EFLG Register Flags.
 * @brief These definitions hold the flags in the EFLG register.
 * @{
 */
#define PI_MCP2515_EFLG_EWARN 0x01
#define PI_MCP2515_EFLG_RXWAR 0x02
#define PI_MCP2515_EFLG_TXWAR 0x04
#define PI_MCP2515_EFLG_RXEP 0x08
#define PI_MCP2515_EFLG_TXEP 0x10
#define PI_MCP2515_EFLG_TXBO 0x20
#define PI_MCP2515_EFLG_RX0OVR 0x40
#define PI_MCP2515_EFLG_RX1OVR 0x80
#define PI_MCP2515_EFLG_MASK 0xF8
/** @} */

#define PI_MCP2515_RXBSIDL_IDE 0x08
#define PI_MCP2515_RXBSIDL_SRR 0x10

/**
 * @defgroup piMCP2515_rx_status `RX STATUS` SPI command flags.
 * @brief These definitions hold the flags in the result of an `RX STATUS` SPI command.
 * @{
 */
#define PI_MCP2515_RX_STATUS_RCV_RXB0 0x40 /**< @brief RXB0 has a message. */
#define PI_MCP2515_RX_STATUS_RCV_RXB1 0x80 /**< @brief RXB1 has a message. */
#define PI_MCP2515_RX_STATUS_RCV_ALL 0xc0 /**< @brief Both RX buffers have messages. A bitwise OR of both RXBn flags. */
#define PI_MCP2515_RX_STATUS_RTR 0x08
#define PI_MCP2515_RX_STATUS_EID 0x10
/** @} */

/* Status Definitions */
#define PI_MCP2515_STATUS_RX0BF 0x01
#define PI_MCP2515_STATUS_RX1BF 0x02
#define PI_MCP2515_STATUS_TX0IF 0x08
#define PI_MCP2515_STATUS_TX1IF 0x20
#define PI_MCP2515_STATUS_TX2IF 0x80

/**
 * @defgroup piMCP2515_canintf CANINTF Register Flags.
 * @brief These definitions hold the flags in the CANINTF register.
 * @{
 */
#define PI_MCP2515_CANINTF_RX0 0x01
#define PI_MCP2515_CANINTF_RX1 0x02
#define PI_MCP2515_CANINTF_TX0IF 0x04
#define PI_MCP2515_CANINTF_TX1IF 0x08
#define PI_MCP2515_CANINTF_TX2IF 0x10
#define PI_MCP2515_CANINTF_ERRIF 0x20 /**< @brief Error interrupt flag. */
/** @} */

typedef enum {
	PI_MCP2515_TXB0 = 0,
	PI_MCP2515_TXB1 = 1,
	PI_MCP2515_TXB2 = 2,
} mcp2515_txb_t;

typedef enum {
	PI_MCP2515_RXB0 = 0,
	PI_MCP2515_RXB1 = 1,
} mcp2515_rxb_t;

typedef enum {
	PI_MCP2515_RXM0 = 0,
	PI_MCP2515_RXM1 = 1,
} mcp2515_rxm_t;

typedef enum {
	PI_MCP2515_RXF0 = 0,
	PI_MCP2515_RXF1 = 1,
	PI_MCP2515_RXF2 = 2,
	PI_MCP2515_RXF3 = 3,
	PI_MCP2515_RXF4 = 4,
	PI_MCP2515_RXF5 = 5,
} mcp2515_rxf_t;

/* CAN frame Mask Definitions */
#define PI_MCP2515_CAN_ID_SFF_MASK 0x000007FFUL
#define PI_MCP2515_CAN_ID_EFF_MASK 0x1FFFFFFFUL
#define PI_MCP2515_CAN_DLC_RTR_FLAG 0x40
#define PI_MCP2515_CAN_DLC_RTR_MASK 0x0F

#define MCP2515_REQOP_CHANGE_SLEEP_CYCLES 128 /**< @brief Number of sleep cycles to wait after REQOP change. */

typedef struct pi_mcp2515 pi_mcp2515_t;

uint32_t	mcp2515_can_id_build(uint32_t, bool);
int		mcp2515_can_clear_txif(pi_mcp2515_t *, uint8_t);
int		mcp2515_can_message_send(pi_mcp2515_t *, const pi_mcp2515_can_frame_t *);
int		mcp2515_can_message_read(pi_mcp2515_t *, pi_mcp2515_can_frame_t *);
int		mcp2515_can_message_read_rxb(pi_mcp2515_t *, mcp2515_rxb_t, pi_mcp2515_can_frame_t *);
bool		mcp2515_can_message_received(pi_mcp2515_t *);
void		mcp2515_rts(pi_mcp2515_t *, uint8_t);

uint8_t		mcp2515_interrupts_get(pi_mcp2515_t *);
uint8_t		mcp2515_interrupts_mask(pi_mcp2515_t *);
void		mcp2515_interrupts_clear(pi_mcp2515_t *);

int	mcp2515_filter(pi_mcp2515_t *, mcp2515_rxf_t, uint32_t, bool);
int	mcp2515_filter_mask(pi_mcp2515_t *, mcp2515_rxm_t, uint32_t, bool);
int	mcp2515_filter_enable(pi_mcp2515_t *, bool);
int	mcp2515_filter_enable_rxb(pi_mcp2515_t *, mcp2515_rxb_t, bool);

int		mcp2515_register_read(pi_mcp2515_t *, uint8_t *, uint8_t, mcp2515_rgstr_t);
int		mcp2515_register_write(pi_mcp2515_t *, uint8_t[], uint8_t, mcp2515_rgstr_t);
int		mcp2515_register_bitmod(pi_mcp2515_t *, uint8_t, uint8_t, mcp2515_rgstr_t);

int		mcp2515_reset(pi_mcp2515_t *);
int		mcp2515_reqop(pi_mcp2515_t *, mcp2515_reqop_t);
mcp2515_reqop_t	mcp2515_reqop_get(pi_mcp2515_t *);

uint8_t		mcp2515_status(pi_mcp2515_t *);
uint8_t		mcp2515_error_tx_count(pi_mcp2515_t *);
uint8_t		mcp2515_error_rx_count(pi_mcp2515_t *);
uint8_t		mcp2515_error_flags(pi_mcp2515_t *);
bool		mcp2515_error(pi_mcp2515_t *);
int		mcp2515_error_clear_errif(pi_mcp2515_t *);

void		mcp2515_micro_sleep(uint64_t micro_s);
uint64_t	mcp2515_osc_time(const pi_mcp2515_t *, uint32_t);

int		mcp2515_bitrate_default_16mhz_1000kbps(pi_mcp2515_t *);
int		mcp2515_bitrate_default_8mhz_500kbps(pi_mcp2515_t *);
int		mcp2515_bitrate_simplified(pi_mcp2515_t *, uint16_t);
int		mcp2515_bitrate_full_optional(pi_mcp2515_t *, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
    bool, bool, bool, bool);
int		mcp2515_reset(pi_mcp2515_t *);

void	mcp2515_free(pi_mcp2515_t *);
int	mcp2515_init(pi_mcp2515_t **, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t, uint8_t);
int	mcp2515_cnf_set(pi_mcp2515_t *, uint8_t, uint8_t, uint8_t);
uint8_t	mcp2515_cnf_get(pi_mcp2515_t *, uint8_t);

void	mcp2515_conf_spi_devpath(pi_mcp2515_t *, char *);
void	mcp2515_conf_gpio_devpath(pi_mcp2515_t *, char *);

void	mcp2515_debug_enable(pi_mcp2515_t *, void (*)(char *, va_list));

#endif /* PIMCP2515_PI_MCP2515_H */