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

#include "pi_MCP2515_handle.h"

#ifdef USE_PICO_LIB
#include "pico/time.h"
#else
#include <unistd.h>
#endif

#include "time.h"

inline void
mcp2515_micro_sleep(uint64_t micro_s)
{
#ifdef USE_PICO_LIB
	sleep_us(micro_s);
#else
	usleep(micro_s);
#endif
}

/**
 * @brief Calculate the time for the number of oscillator cycles supplied, and based on the oscillator frequency.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param num_cycles The number of oscillator cycles to calculate time for.
 * @return The calculated time in microseconds.
 */
inline uint64_t
mcp2515_osc_time(const pi_mcp2515_t *pi_mcp2515, uint32_t num_cycles)
{
	uint64_t cycle_len_nano_sec;

	cycle_len_nano_sec = 1000000000L / (pi_mcp2515->osc_mhz * 1000000L);

	return (num_cycles * cycle_len_nano_sec / 1000L); /* return microseconds */
}
