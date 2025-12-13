/*
 * Created by Roos Catling-Tate.
 * 
 * Copyright 2025
 */

#include "gpio.h"

#include <stdint.h>
#include <hardware/spi.h>

#include "pi_MCP2515.h"

#ifdef USE_PICO_LIB

spi_inst_t	*spi_inst_from_index(uint8_t);

spi_inst_t *
spi_inst_from_index(uint8_t index)
{
	switch (index) {
	case 0:
		return (spi0);
	case 1:
		return (spi1);
	default:
		return (NULL);
	}
}

#endif /* USE_PICO_LIB */

void
mcp2515_gpio_function_set(uint8_t pin, uint8_t function)
{
#ifdef USE_PICO_LIB
	gpio_set_function(pin, function);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_init(uint8_t pin)
{
#ifdef USE_PICO_LIB
	gpio_init(pin);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_spi_init(uint8_t spi_channel, uint32_t baudrate)
{
#ifdef USE_PICO_LIB
	spi_inst_t *spi_inst = spi_inst_from_index(spi_channel);

	spi_init(spi_inst, baudrate);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_set_dir(uint8_t gpio, bool out)
{
#ifdef USE_PICO_LIB
	gpio_set_dir(gpio, out);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	spi_write_blocking(spi_inst_from_index(pi_mcp2515->spi_channel), data, len);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	/* Note: For now, repeated_tx_data is not used anywhere in the library so we just skip it. */
	spi_read_blocking(spi_inst_from_index(pi_mcp2515->spi_channel), 0x00, data, len);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}

void
mcp2515_gpio_put(uint8_t pin, uint8_t value)
{
#ifdef USE_PICO_LIB
	gpio_put(pin, value);
#elifdef USE_PRINT_DEBUG
	/* TODO */
#endif
}
