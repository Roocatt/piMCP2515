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

/* This is excluded from Doxygen as this is only used for internal functionality.
 *
 * This is a nightmare C file from hell with about as much C preprocessor as actual C code. I may work on making this
 * better, but this file exists to add a shim between the actual GPIO/SPI backing so it may just be used to isolate the
 * horrors from the rest of the project.
 */

#include <stdio.h>
#ifdef USE_PICO_LIB

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#elif defined(USE_SPIDEV)

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

/* TODO configurable cross-compile? BSD compiler setup in cmake? Differences across BSDs? */
#if defined(__linux__)
#define USE_SPIDEV_LINUX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define USE_SPIDEV_BSD
#else
#error "Unsupported OS"
#endif

#elif defined(USE_PRINT_DEBUG)
#include <stdio.h>
#endif

#ifdef USE_SPIDEV_LINUX
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#elif defined(USE_SPIDEV_BSD)
#include <dev/gpio/gpio.h>
#include <dev/spi/spi_io.h>
#endif

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "pi_MCP2515_handle.h"

#include "gpio.h"

#ifdef USE_SPIDEV
static int	spidev_duplex_com(const pi_mcp2515_t *, const char[sizeof(uint64_t)], const char[sizeof(uint64_t)]);

/**
 * @brief Perform a round of full duplex communication over SPI.
 *
 * spidev only does duplex communication. So, we split that off to this function.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param tx_buffer the buffer to use for transmitting.
 * @param rx_buffer the buffer to use for receiving.
 * @return zero if success, otherwise non-zero.
 */
static int
spidev_duplex_com(const pi_mcp2515_t *pi_mcp2515, const char tx_buffer[sizeof(uint64_t)], const char rx_buffer[sizeof(uint64_t)])
{
	struct spi_ioc_transfer tr = {
		.tx_buf = *(uint64_t *)tx_buffer,
		.rx_buf = *(uint64_t *)rx_buffer,
		.len = sizeof(uint64_t),
		.delay_usecs = pi_mcp2515->gpio_spi_delay_usec,
		.speed_hz = pi_mcp2515->spi_clock,
		.bits_per_word = pi_mcp2515->gpio_spi_bits_per_word,
	};

	return (ioctl(pi_mcp2515->gpio_spidev_fd, SPI_IOC_MESSAGE(1), &tr));
}
#endif /* USE_SPIDEV */

/**
 * @brief Set up a GPIO pin.
 *
 * When compiled for Pico, it will simply call `gpio_init(pin)`.
 *
 * With spidev, this will open a file descriptor for the specified pin to prepare it for future use.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param pin the GPIO pin to set up.
 */
int
mcp2515_gpio_init(pi_mcp2515_t *pi_mcp2515, uint8_t pin)
{
#ifdef USE_PICO_LIB
	gpio_init(pin);

	return (0);
#elif defined(USE_SPIDEV)
	struct gpio_v2_line_request rq = { 0 };
	int res;

	rq.offsets[0] = pin;
	rq.num_lines = 1;
	rq.config.flags = GPIOHANDLE_REQUEST_INPUT;
	rq.config.num_attrs = 0;
	strncpy(rq.consumer, "pi_mcp2515", sizeof(rq.consumer));

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	if (!res)
		pi_mcp2515->gpio_pin_fd_map[pin] = rq.fd;

	return (res);
#elif defined(USE_SPIDEV_BSD)
	struct gpio_pin_set pin_config = { 0 };

	snprintf(pin_config.gp_name, GPIOPINMAXNAME, "pi_mcp2515 pin #%d", pin);
	pin_config.gp_flags = GPIO_PIN_INPUT;

	return (ioctl(pi_mcp2515->gpio_gpio_fd, GPIOPINSET, &pin_config));
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_init(0x%02x)\n", pin);

	return (0);
#endif
}

/**
 * @brief Clean up everything GPIO for the specified handle.
 *
 * Currently, this is a NOP except for when using spidev.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 */
void
mcp2515_gpio_spi_free(const pi_mcp2515_t *pi_mcp2515)
{
#ifdef USE_SPIDEV
	if (pi_mcp2515->gpio_spidev_fd > 0)
		close(pi_mcp2515->gpio_spidev_fd);

	if (pi_mcp2515->gpio_gpio_fd > 0)
		close(pi_mcp2515->gpio_gpio_fd);

	for (uint8_t i = 0; i < PI_MCP2515_GPIO_PIN_MAP_LEN; i++)
		if (pi_mcp2515->gpio_pin_fd_map[i] > 0)
			close(pi_mcp2515->gpio_pin_fd_map[i]);
#endif
}

int
mcp2515_gpio_spi_init(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_gpio_spi_init_full_optional(pi_mcp2515, 0, 8));
}

int
mcp2515_gpio_spi_init_full_optional(pi_mcp2515_t *pi_mcp2515, uint8_t mode, uint8_t bits_per_word)
{
#ifdef USE_PICO_LIB
	spi_inst_t *spi_inst;

	switch (pi_mcp2515->spi_channel) {
	case 0:
		spi_inst = spi0;
		break;
	case 1:
		spi_inst = spi1;
		break;
	default:
		return (-1);
	}

	pi_mcp2515->gpio_spi_inst = spi_inst,

	spi_init(spi_inst, pi_mcp2515->spi_clock);
	gpio_set_function(pi_mcp2515->tx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->rx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->sck_pin, PI_MCP2515_GPIO_FUNC_SPI);
	mcp2515_gpio_init(pi_mcp2515, pi_mcp2515->cs_pin);

	mcp2515_gpio_set_dir(pi_mcp2515, pi_mcp2515->cs_pin, true);

	return (0);
#elif defined(USE_SPIDEV)
#ifdef USE_SPIDEV_BSD
	spi_ioctl_configure_t spi_cfg = { 0 };
#endif
	int res, spidev_fd, gpio_fd;
	char *spidev_path;

	if (pi_mcp2515->gpio_dev_spi_path == NULL) {
		if (pi_mcp2515->spi_channel == 0)
			spidev_path = "/dev/spi0.0";
		else /* pi_mcp2515->spi_channel == 1 */
			spidev_path = "/dev/spi0.1";
	} else
		spidev_path = pi_mcp2515->gpio_dev_spi_path;

	gpio_fd = open(pi_mcp2515->gpio_dev_gpio_path == NULL ? "/dev/gpio0" : pi_mcp2515->gpio_dev_gpio_path, O_RDWR);
	if (gpio_fd < 0)
		return (-1);
	spidev_fd = open(spidev_path, O_RDWR);
	if (spidev_fd < 0) {
		close(gpio_fd);
		res = -1;
		goto err;
	}

#ifdef USE_SPIDEV_LINUX
	memset(&pi_mcp2515->gpio_pin_fd_map, 0 , sizeof(pi_mcp2515->gpio_pin_fd_map));

	if ((res = ioctl(spidev_fd, SPI_IOC_WR_MODE, &mode)
	    || ioctl(spidev_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word)
	    || ioctl(spidev_fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word)
	    || ioctl(spidev_fd, SPI_IOC_WR_MAX_SPEED_HZ, &pi_mcp2515->spi_clock)
	    || ioctl(spidev_fd, SPI_IOC_RD_MAX_SPEED_HZ, &pi_mcp2515->spi_clock))) {
		close(gpio_fd);
		close(spidev_fd);
		goto err;
	}
#elif defined(USE_SPIDEV_BSD)
	spi_cfg.sic_mode = pi_mcp2515->gpio_spi_mode
	spi_cfg.sic_speed = pi_mcp2515->spi_clock;

	if ((res = ioctl(spidev_fd, SPI_IOCTL_CONFIGURE, &spi_cfg))) {
		close(gpio_fd);
		close(spidev_fd);
		goto err;
	}
#endif

	pi_mcp2515->gpio_gpio_fd = gpio_fd;
	pi_mcp2515->gpio_spidev_fd = spidev_fd;
	pi_mcp2515->gpio_spi_mode = mode;
	pi_mcp2515->gpio_spi_bits_per_word = bits_per_word;
	pi_mcp2515->gpio_spi_delay_usec = 0;

err:
	return (res);
#elif defined(USE_PRINT_DEBUG)
	printf("spi_init(0x%02x, 0x%08x)\n", pi_mcp2515->spi_channel, pi_mcp2515->spi_clock);
	return (0);
#endif
}

int
mcp2515_gpio_set_dir(const pi_mcp2515_t *pi_mcp2515, uint8_t gpio, bool out)
{
#ifdef USE_PICO_LIB
	gpio_set_dir(gpio, out);

	return (0);
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_set_dir(0x%02x, %s)\n", gpio, out ? "true" : "false");

	return (0);
#elif defined(USE_SPIDEV_LINUX)
	struct gpio_v2_line_config config = { 0 };

	if (out)
		config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
	else
		config.flags = GPIO_V2_LINE_FLAG_INPUT;

	return (ioctl(pi_mcp2515->gpio_pin_fd_map[gpio], GPIO_V2_LINE_SET_CONFIG_IOCTL, &config));
#elif defined(USE_SPIDEV_BSD)
	/* TODO Confirm behaviour with second GPIOSET on things like name. */
	struct gpio_pin_set pin_config = { 0 };

	pin_config.gp_pin = pin_number;
	if (out)
		pin_config.gp_flags = GPIO_PIN_OUTPUT;
	else
		pin_config.gp_flags = GPIO_PIN_INPUT;

	return (ioctl(pi_mcp2515->gpio_gpio_fd, GPIOPINSET, &pin_config));
#endif
}

int
mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	spi_write_blocking(pi_mcp2515->gpio_spi_inst, data, len);

	return (0);
#elif defined(USE_SPIDEV)
	/* TODO incomplete */
	int res, tail_len;
	char tx_buffer[sizeof(uint64_t)] = { 0 }, rx_buffer[sizeof(tx_buffer)] = { 0 };

	for (uint8_t i = 0; i < len; i += sizeof(uint64_t)) {
		if (len - i > sizeof(uint64_t))
			memcpy(&tx_buffer, &data[i], sizeof(tx_buffer));
		else {
			tail_len = len - i;
			memcpy(&tx_buffer, &data[i], tail_len);
			memset(&tx_buffer[tail_len], 0, sizeof(tx_buffer) - tail_len);
		}
		if ((res = spidev_duplex_com(pi_mcp2515, tx_buffer, rx_buffer)))
			return (res);
	}

	return (0);
#elif defined(USE_PRINT_DEBUG)
	printf("spi_write_blocking(0x%02x, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
	    len == 1 ? "" : "...", len);

	return (0);
#endif
}

int
mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	/* Note: For now, repeated_tx_data is not used anywhere in the library so we just skip it. */
	spi_read_blocking(pi_mcp2515->gpio_spi_inst, 0x00, data, len);

	return (0);
#elif defined(USE_SPIDEV)
	/* TODO incomplete */
	int res;
	char tx_buffer[sizeof(uint64_t)] = { 0 }, rx_buffer[sizeof(tx_buffer)] = { 0 };

	for (uint8_t i = 0; i < len; i += sizeof(uint64_t)) {
		if ((res = spidev_duplex_com(pi_mcp2515, tx_buffer, rx_buffer)))
			return (res);
		memcpy(&data[i], &rx_buffer, len - i > sizeof(uint64_t) ? sizeof(rx_buffer) : len - i);
	}
	return (0);
#elif defined(USE_PRINT_DEBUG)
	printf("spi_read_blocking(0x%02x, 0x00, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
	    len == 1 ? "" : "...", len);

	return (0);
#endif
}

int
mcp2515_gpio_put(const pi_mcp2515_t *pi_mcp2515, uint8_t pin, uint8_t value)
{
#ifdef USE_PICO_LIB
	gpio_put(pin, value);

	return (0);
#elif defined(USE_SPIDEV)
	struct gpio_v2_line_values values;

	values.mask = (1 << (pin - 1));
	values.bits = value ? 1 : 0;

	return (ioctl(pi_mcp2515->gpio_pin_fd_map[pin], GPIO_V2_LINE_SET_VALUES_IOCTL, &values));
#elif defined(USE_SPIDEV_BSD)
	struct gpio_pin_op pin_op;

	pin_op.gp_pin = pin;
	pin_op.gp_value = value ? 1 : 0;

	return (ioctl(pi_mcp2515->gpio_gpio_fd, GPIOPINWRITE, &pin_op));
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_put(0x%02x, 0x%02x)\n", pin, value);

	return (0);
#endif
}

/**
 * @brief Calculate the time for the number of oscillator cycles supplied, and based on the oscillator frequency.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param num_cycles The number of oscillator cycles to calculate time for.
 * @return The calculated time in microseconds.
 */
uint64_t
mcp2515_osc_time(const pi_mcp2515_t *pi_mcp2515, uint32_t num_cycles)
{
	uint64_t cycle_len_nano_sec;

	cycle_len_nano_sec = 1000000000 / (pi_mcp2515->osc_mhz * 1000000);

	return (num_cycles * cycle_len_nano_sec / 1000); /* return microseconds */
}
