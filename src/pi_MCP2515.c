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

/* Warning, this is still an early work in progress.
 * There is a lot of bad/untested code here. Please be patient, I'm working on it...
 */

#include <stdbool.h>

#include "pi_MCP2515_defs.h"
#include "registers.h"
#include "gpio.h"

#include "pi_MCP2515.h"

/**
 * @defgroup piMCP2515_config_init_functions Init and Config Functions
 * @brief These functions handle initialization and configuration.
 * @{
 */
/**
 * @brief A simplified bitrate setting function.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param baud_rate_kbps the baud rate to use in kbps. Commonly either 500 or 1000.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_bitrate_simplified(pi_mcp2515_t *pi_mcp2515, uint16_t baud_rate_kbps)
{
	return (mcp2515_bitrate_full_optional(pi_mcp2515, baud_rate_kbps, 2, 0, 2, 2,
	    3, false, false, false, true));
}

/**
 * @brief Configure the bitrate and all other CNF register stored parameters.
 *
 * This function accepts all possible configurable
 * options, while `mcp2515_bitrate_simplified` can be used for a simplified process.
 *
 * TODO first draft. Test/validate/fix. Make sure this is right.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param baud_rate_kbps the baud rate to use in kbps. Commonly either 500 or 1000.
 * @param sjw the synchronization jump width in Tqs (1-4).
 * @param prescaler the prescaler divisor (0-63)
 * @param prseg_tqps the prop segment Tqs/segment
 * @param phseg_tqps1 the phase segment 1 Tqs/segment
 * @param phseg_tqps2 the phase segment 2 Tqs/segment
 * @param sof if start-of-frame signal is enabled.
 * @param wakfil if wakeup filter should be enabled.
 * @param sam is bus line should be sampled 3 times at sample point (otherwise it is sampled once).
 * @param btlmode if BTLMODE is enabled.
 *                true:  PS2 length is determined by PHSEG2[2:0] (PHSEG2 values originate from @p phseg_tqps2).
 *                false: PS2 length is the greater of PS1 and IPT (2 Tqs)
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_bitrate_full_optional(pi_mcp2515_t *pi_mcp2515, uint16_t baud_rate_kbps, uint8_t sjw, uint8_t prescaler,
    uint8_t prseg_tqps, uint8_t phseg_tqps1, uint8_t phseg_tqps2, bool sof, bool wakfil, bool sam, bool btlmode)
{
	int res;
	uint8_t cnf1, cnf2, cnf3;

	res = 0;

	if (baud_rate_kbps > 1000 || baud_rate_kbps == 0 || sjw > 4 || sjw == 0 || prescaler > 63 || prseg_tqps == 0
	    || prseg_tqps > 8 || phseg_tqps1 == 0 || phseg_tqps1 > 8 || phseg_tqps2 == 0 || phseg_tqps2 > 8
	    || phseg_tqps2 <= sjw || prseg_tqps + phseg_tqps1 < phseg_tqps2) {
		res = 1;
		goto err;
	}

	cnf1 = ((sjw - 1) << 6) | (prescaler & 0x3f);
	cnf2 = (((phseg_tqps1 - 1) & 0x07) << 3) | (prseg_tqps & 0x7);
	cnf3 = (phseg_tqps2 - 1) & 0x07;

	if (btlmode)
		cnf2 |= 0x80;
	if (sam)
		cnf2 |= 0x40;
	if (sof)
		cnf3 |= 0x80;
	if (wakfil)
		cnf3 |= 0x40;

	if ((res = mcp2515_register_write(pi_mcp2515, &cnf1, 1, PI_MCP2515_RGSTR_CNF1)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, &cnf2, 1, PI_MCP2515_RGSTR_CNF2)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, &cnf3, 1, PI_MCP2515_RGSTR_CNF3);

err:
	return (res);
}

/**
 * @brief Set the values of all CNF register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param cnf1 the value to use for the CNF1 register.
 * @param cnf2 the value to use for the CNF2 register.
 * @param cnf3 the value to use for the CNF3 register.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_cnf_set(pi_mcp2515_t *pi_mcp2515, uint8_t cnf1, uint8_t cnf2, uint8_t cnf3)
{
	int res;

	if ((res = mcp2515_register_write(pi_mcp2515, &cnf1, 1, PI_MCP2515_RGSTR_CNF1)))
		goto err;
	if ((res = mcp2515_register_write(pi_mcp2515, &cnf2, 1, PI_MCP2515_RGSTR_CNF2)))
		goto err;
	res = mcp2515_register_write(pi_mcp2515, &cnf3, 1, PI_MCP2515_RGSTR_CNF3);

	err:
		return (res);
}

/**
 * @brief Get the value of a CNF register.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param cnf The CNF register index such that `cnf` being `1` will fetch from CNF1, etc.
 * @return The value of the register.
 */
uint8_t
mcp2515_cnf_get(pi_mcp2515_t *pi_mcp2515, uint8_t cnf)
{
	uint8_t res = 0, rgstr;

	/* CNF register numbers aren't zero indexed for some indiscernible reason. Start with 1 here for consistency. */
	switch (cnf) {
	case 1:
		rgstr = PI_MCP2515_RGSTR_CNF1;
		break;
	case 2:
		rgstr = PI_MCP2515_RGSTR_CNF2;
		break;
	case 3:
		rgstr = PI_MCP2515_RGSTR_CNF3;
		break;
	default:
		goto err;
	}

	mcp2515_register_read(pi_mcp2515, &res, 1, rgstr);

err:
	return (res);
}

/**
 * @brief Setup and prepare a pi_mcp2515_t structure based on the parameters provided.
 *
 * This will still require one of the bitrate setting functions or mcp2515_cnf_set to be used to set up the CNF
 * registers.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 *
 * @param spi_clock the frequency to use for SPI communication in Hz.
 * @param osc_mhz the frequency of the MCP2515 oscillator in MHz.
 * @param spi_channel the SPI channel to use which may be `0` or `1`.
 * @param cs_pin the GPIO pin to use for SPI chip select.
 * @return zero if success, otherwise non-zero
 */
int
mcp2515_init(pi_mcp2515_t *pi_mcp2515, uint8_t spi_channel, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin,
    uint8_t sck_pin, uint32_t spi_clock, uint8_t osc_mhz)
{
	int res;

	if (osc_mhz > 40|| osc_mhz == 0) {
		res = 1;
		goto err;
	}

	pi_mcp2515->spi_channel = spi_channel;
	pi_mcp2515->cs_pin = cs_pin;

	/* TODO these are only used for Pico. Figure out a cleaner way. */
	pi_mcp2515->sck_pin = sck_pin;
	pi_mcp2515->tx_pin = tx_pin;
	pi_mcp2515->rx_pin = rx_pin;

	pi_mcp2515->spi_clock = spi_clock;
	pi_mcp2515->osc_mhz = osc_mhz;

	if ((res = mcp2515_gpio_spi_init(pi_mcp2515)))
		goto err;

	UNSET_CS(pi_mcp2515);

err:
	return (res);
}

/**
 * @brief Cleanup after everything.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 */
void
mcp2515_free(const pi_mcp2515_t *pi_mcp2515)
{
	mcp2515_gpio_spi_free(pi_mcp2515);
}
/** @} */
