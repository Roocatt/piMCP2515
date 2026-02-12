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

#ifndef __PIMCP2515_PIMCP2515_PICO_SH_H__
#define __PIMCP2515_PIMCP2515_PICO_SH_H__

#define PICO_SH_PROMPT "piMCP2515> "

#define MAX_COMMAND_LENGTH 100

#define CMD_REQOP "reqop "
#define CMD_REQOP_LEN sizeof(CMD_REQOP)
#define CMD_REQOP_LOOPBACK "loopback"
#define CMD_REQOP_LOOPBACK_LEN sizeof(CMD_REQOP_LOOPBACK)
#define CMD_REQOP_NORMAL "normal"
#define CMD_REQOP_NORMAL_LEN sizeof(CMD_REQOP_NORMAL)
#define CMD_REQOP_CONFIG "config"
#define CMD_REQOP_CONFIG_LEN sizeof(CMD_REQOP_CONFIG)
#define CMD_REQOP_SLEEP "sleep"
#define CMD_REQOP_SLEEP_LEN sizeof(CMD_REQOP_SLEEP)
#define CMD_REQOP_GET "get"
#define CMD_REQOP_GET_LEN sizeof(CMD_REQOP_GET)

#define CMD_CFG "cfg "
#define CMD_CFG_LEN sizeof(CMD_CFG)
#define CMD_CFG_DUMP "dump"
#define CMD_CFG_DUMP_LEN sizeof(CMD_CFG_DUMP)

#define CMD_CAN "can "
#define CMD_CAN_LEN sizeof(CMD_CAN)
#define CMD_CAN_RECEIVED "recieved"
#define CMD_CAN_RECEIVED_LEN sizeof(CMD_CAN_RECEIVED)
#define CMD_CAN_SEND "send"
#define CMD_CAN_SEND_LEN sizeof(CMD_CAN_SEND)
#define CMD_CAN_READ "read"
#define CMD_CAN_READ_LEN sizeof(CMD_CAN_READ)

#define CMD_STATUS "status"
#define CMD_STATUS_LEN sizeof(CMD_STATUS)

#define CMD_RESET "reset"
#define CMD_RESET_LEN sizeof(CMD_RESET)

#define BAD_CMD_ERR "invalid command: %s\n"
#define BAD_CMD_PRINT(x) printf(BAD_CMD_ERR, x)

#define PRINT_RES(x) printf(#x " has returned: %d\n", x)

#endif /* __PIMCP2515_PIMCP2515_PICO_SH_H__ */