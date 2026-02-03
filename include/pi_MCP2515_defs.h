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
/* This header is used to hold important definitions. It is used both internal and external to the library.
 */

#ifndef PIMCP2515_PI_MCP2515_DEFS_H
#define PIMCP2515_PI_MCP2515_DEFS_H

/**
 * @defgroup piMCP2515_register_addresses Register Addresses
 * @brief These definitions hold the MCP2515 register addresses.
 * @{
 */
#define PI_MCP2515_RGSTR_RXF0SIDH 0x00
#define PI_MCP2515_RGSTR_RXF1SIDH 0x04
#define PI_MCP2515_RGSTR_RXF2SIDH 0x08
#define PI_MCP2515_RGSTR_RXF3SIDH 0x10
#define PI_MCP2515_RGSTR_RXF4SIDH 0x14
#define PI_MCP2515_RGSTR_RXF5SIDH 0x18
#define PI_MCP2515_RGSTR_RXM0SIDH 0x20
#define PI_MCP2515_RGSTR_RXM1SIDH 0x24
#define PI_MCP2515_RGSTR_CANSTAT 0x0E
#define PI_MCP2515_RGSTR_CANCTRL 0x0F
#define PI_MCP2515_RGSTR_CANINTE 0x2B
#define PI_MCP2515_RGSTR_CANINTF 0x2C
#define PI_MCP2515_RGSTR_EFLG 0x2D
#define PI_MCP2515_RGSTR_ECRX 0x1D
#define PI_MCP2515_RGSTR_ECTX 0x1C
#define PI_MCP2515_RGSTR_CNF1 0x2A
#define PI_MCP2515_RGSTR_CNF2 0x29
#define PI_MCP2515_RGSTR_CNF3 0x28
#define PI_MCP2515_RGSTR_RX0CTRL 0x60
#define PI_MCP2515_RGSTR_RX0SIDH 0x61
#define PI_MCP2515_RGSTR_RX0DATA 0x66
#define PI_MCP2515_RGSTR_RX1CTRL 0x70
#define PI_MCP2515_RGSTR_RX1SIDH 0x71
#define PI_MCP2515_RGSTR_RX1DATA 0x76
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
#define PI_MCP2515_INSTR_RESET 0xC0 /**< @brief SPI interface status instruction. */

#define PI_MCP2515_INSTR_LOAD_TX0 0x40
#define PI_MCP2515_INSTR_LOAD_TX1 0x42
#define PI_MCP2515_INSTR_LOAD_TX2 0x44

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
#define PI_MCP2515_REQOP_NORMAL 0x00 /**< @brief Normal operating mode. */
#define PI_MCP2515_REQOP_SLEEP 0x20 /**< @brief Sleep operating mode. */
#define PI_MCP2515_REQOP_LOOPBACK 0x40 /**< @brief Loopback operating mode. */
#define PI_MCP2515_REQOP_LISTENONLY 0x60 /**< @brief Listenonly operating mode. */
#define PI_MCP2515_REQOP_CONFIG 0x80 /**< @brief Config operating mode. */
#define PI_MCP2515_REQOP_POWERUP 0xE0 /**< @brief Powerup operating mode. */
#define PI_MCP2515_REQOP_MASK 0xE0 /**< @brief Mask for REQOP values. */
/** @} */

/* CTRL Definitions */
#define PI_MCP2515_CTRL_RTR 0x08
#define PI_MCP2515_CTRL_TXREQ 0x08
#define PI_MCP2515_CTRL_TXERR 0x10
#define PI_MCP2515_CTRL_MLOA 0x20
#define PI_MCP2515_CTRL_ABTF 0x40

/* Flag Definitions */
#define PI_MCP2515_FLAG_RTR 0x40000000UL
#define PI_MCP2515_FLAG_EFF 0x80000000UL

/* Error Flag Definitions */
#define PI_MCP2515_EFLG_EWARN 0x01
#define PI_MCP2515_EFLG_RXWAR 0x02
#define PI_MCP2515_EFLG_TXWAR 0x04
#define PI_MCP2515_EFLG_RXEP 0x08
#define PI_MCP2515_EFLG_TXEP 0x10
#define PI_MCP2515_EFLG_TXBO 0x20
#define PI_MCP2515_EFLG_RX0OVR 0x40
#define PI_MCP2515_EFLG_RX1OVR 0x80
#define PI_MCP2515_EFLG_MASK 0xF8

/* RX/TX Status Definitions */
#define PI_MCP2515_STATUS_RX0BF 0x01
#define PI_MCP2515_STATUS_RX1BF 0x02

/* CANINTF Definitions */
#define PI_MCP2515_CANINTF_RX0 0x01
#define PI_MCP2515_CANINTF_RX1 0x02
#define PI_MCP2515_CANINTF_ERRIF 0x20 /**< @brief Error interrupt flag. */

/* ID Mask Definitions */
#define PI_MCP2515_ID_MASK_SFF 0x000007FFUL
#define PI_MCP2515_ID_MASK_EFF 0x1FFFFFFFUL

#define PI_MCP2515_RTR_MASK 0x40

#endif /* PIMCP2515_PI_MCP2515_DEFS_H */