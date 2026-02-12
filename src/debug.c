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

#ifndef NO_DEBUG

#include <stdarg.h>
#include <stdio.h>

#include <pi_MCP2515.h>

#include "internal.h"

static void	mcp2515_debug_default_callback(char *, va_list);

static void
mcp2515_debug_default_callback(char *msg, va_list args)
{
	printf("libpiMCP2515: ");
	vprintf(msg, args);
}

void
__mcp2515_debug(pi_mcp2515_t *pi_mcp2515, char *msg, ...)
{
	va_list args;

	if (pi_mcp2515->callback == NULL)
		return;

	va_start(args, msg);
	pi_mcp2515->callback(msg, args);
	va_end(args);
}
#endif /* NO_DEBUG */

/**
 * Enable debug logging.
 *
 * NOOP if compiled with NO_DEBUG defined.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param callback the function to call for debugging, or NULL to use the default stdout logging.
 */
void
mcp2515_debug_enable(pi_mcp2515_t *pi_mcp2515, void (*callback)(char *, va_list))
{
#ifndef NO_DEBUG
	if (callback == NULL)
		pi_mcp2515->callback = mcp2515_debug_default_callback;
	else
		pi_mcp2515->callback = callback;
#endif
}
