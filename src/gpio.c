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

/* This is a nightmare C file from hell with about as much C preprocessor as actual C code. I may work on making this
 * better, but this file exists to add a shim between the actual GPIO/SPI backing so it may just be used to isolate the
 * horrors from the rest of the project.
 */

#ifdef USE_PICO_LIB

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#elif defined(USE_SPIDEV)

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(__linux__)
#define USE_SPIDEV_LINUX
#else
/* TODO Support other Pi OS options (BSDs). Ex: defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) */
#error "Unsupported OS"
#endif

#elif defined(USE_PRINT_DEBUG)

#include <stdio.h>

#endif

#ifdef USE_SPIDEV_LINUX
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
/* TODO Add BSD OS support things*/
#endif

#include <string.h>
#include <stdint.h>

#include "pi_MCP2515.h"

#include "gpio.h"

#ifdef USE_SPIDEV

static int	spidev_duplex_com(const pi_mcp2515_t *, char[sizeof(uint64_t)], char[sizeof(uint64_t)]);

/* spidev only does duplex communication. So, we split that off to this function. */
static int
spidev_duplex_com(const pi_mcp2515_t *pi_mcp2515, char tx_buffer[sizeof(uint64_t)], char rx_buffer[sizeof(uint64_t)])
{
	struct spi_ioc_transfer tr = {
		.tx_buf = (uint64_t)tx_buffer,
		.rx_buf = (uint64_t)rx_buffer,
		.len = sizeof(uint64_t),
		.delay_usecs = pi_mcp2515->gpio_spi_delay_usec,
		.speed_hz = pi_mcp2515->spi_clock,
		.bits_per_word = pi_mcp2515->gpio_spi_bits_per_word,
	};

	return (ioctl(pi_mcp2515->gpio_spidev_fd, SPI_IOC_MESSAGE(1), &tr));
}

#endif /* USE_SPIDEV */

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
	rq.config.flags = GPIOHANDLE_REQUEST_OUTPUT;
	rq.config.num_attrs = 0;
	strncpy(rq.consumer, "pi_mcp2515", sizeof(rq.consumer));

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	if (!res) {
		pi_mcp2515->gpio_pin_fd_map[pin] = rq.fd;
	}

	return (res);
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_init(0x%02x)\n", pin);
	return (0);
#endif
}

void
mcp2515_gpio_spi_free(const pi_mcp2515_t *pi_mcp2515)
{
#ifdef USE_PICO_LIB
	/* TODO */
#elif defined(USE_SPIDEV)
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
mcp2515_gpio_spi_init(pi_mcp2515_t *pi_mcp2515, uint8_t spi_channel, uint32_t baud_rate)
{
#ifdef USE_PICO_LIB
	spi_inst_t *spi_inst;

	switch (spi_channel) {
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

	spi_init(spi_inst, baud_rate);
	gpio_set_function(pi_mcp2515->tx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->rx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->sck_pin, PI_MCP2515_GPIO_FUNC_SPI);
	mcp2515_gpio_init(pi_mcp2515, pi_mcp2515->cs_pin);

	/* TODO Sort conditional building with '1u' / GPIO_OUT */
	mcp2515_gpio_set_dir(pi_mcp2515->cs_pin, GPIO_OUT);
	return (0);
#elif defined(USE_SPIDEV)
	int res, spidev_fd, gpio_fd;
	uint8_t mode, bits_per_word;

	memset(&pi_mcp2515->gpio_pin_fd_map, 0 , sizeof(pi_mcp2515->gpio_pin_fd_map));

	mode = 0; /* TODO mode, bits_per_word, and delay_usec value configurable or defaults?*/
	bits_per_word = 8;

	gpio_fd = open("/dev/gpio0", O_RDWR); /* TODO configure path and handle error */
	if (gpio_fd < 0) {
		return (-1);
	}
	spidev_fd = open("/dev/spi0", O_RDWR); /* TODO configure path and handle error */
	if (spidev_fd < 0) {
		close(gpio_fd);
		return (-1);
	}

	if ((res = ioctl(spidev_fd, SPI_IOC_WR_MODE, &mode)
	    || ioctl(spidev_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word)
	    || ioctl(spidev_fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word)
	    || ioctl(spidev_fd, SPI_IOC_WR_MAX_SPEED_HZ, &pi_mcp2515->spi_clock)
	    || ioctl(spidev_fd, SPI_IOC_RD_MAX_SPEED_HZ, &pi_mcp2515->spi_clock))) {
		close(gpio_fd);
		close(spidev_fd);
		return (res);
	}

	pi_mcp2515->gpio_gpio_fd = gpio_fd;
	pi_mcp2515->gpio_spidev_fd = spidev_fd;
	pi_mcp2515->gpio_spi_mode = mode;
	pi_mcp2515->gpio_spi_bits_per_word = bits_per_word;
	pi_mcp2515->gpio_spi_delay_usec = 0;

	return (0);
#elif defined(USE_PRINT_DEBUG)
	printf("spi_init(0x%02x, 0x%08x)\n", spi_channel, baud_rate);
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
#elif defined(USE_SPIDEV)
	struct gpio_v2_line_config config;

	if (out)
		config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
	else
		config.flags = GPIO_V2_LINE_FLAG_INPUT;

	return (ioctl(pi_mcp2515->gpio_pin_fd_map[gpio], GPIO_V2_LINE_SET_CONFIG_IOCTL, &config));
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
		if (len - i > sizeof(uint64_t)) {
			memcpy(&tx_buffer, &data[i], sizeof(tx_buffer));
		} else {
			tail_len = len - i;
			memcpy(&tx_buffer, &data[i], tail_len);
			memset(&tx_buffer[tail_len], 0, sizeof(tx_buffer) - tail_len);
		}
		if ((res = spidev_duplex_com(pi_mcp2515, tx_buffer, rx_buffer))) {
			return (res);
		}
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
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_put(0x%02x, 0x%02x)\n", pin, value);
	return (0);
#endif
}
