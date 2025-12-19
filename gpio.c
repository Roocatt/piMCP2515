/*
 * Created by Roos Catling-Tate.
 * 
 * Copyright 2025
 */

/* This is a nightmare C file from hell with about as much C preprocessor as actual C code. I may work on making this
 * better, but this file exists to add a shim between the actual GPIO/SPI backing so it may just be used to isolate the
 * horrors from the rest of the project.
 */

#ifdef USE_PICO_LIB

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#elifdef USE_SPIDEV

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(__linux__)
#define USE_SPIDEV_LINUX
#else
/* TODO Support other Pi OS options (BSDs). Ex: defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) */
#error "Unsupported OS"
#endif

#elifdef USE_PRINT_DEBUG

#include <stdio.h>

#endif

#ifdef USE_SPIDEV_LINUX
#include <linux/spi/spidev.h>
/* TODO Add BSD OS support things*/
#endif

#include <stdint.h>
#include "pi_MCP2515.h"

#include "gpio.h"

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
mcp2515_gpio_init(uint8_t pin)
{
#ifdef USE_PICO_LIB
	gpio_init(pin);
#elifdef USE_PRINT_DEBUG
	printf("gpio_init(0x%02x)\n", pin);
#endif
}

void
mcp2515_gpio_spi_init(pi_mcp2515_t *pi_mcp2515, uint8_t spi_channel, uint32_t baudrate)
{
	gpio_params_t gpio_params;
#ifdef USE_PICO_LIB
	spi_inst_t *spi_inst;

	switch (index) {
	case 0:
		spi_inst = spi0;
		break;
	case 1:
		spi_inst = spi1;
		break;
	default:
		return; /* TODO handle bad value */
	}

	gpio_params = {
		.spi_inst = spi_inst,
	};

	spi_init(spi_inst, baudrate);
	gpio_set_function(pi_mcp2515->tx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->rx_pin, PI_MCP2515_GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->sck_pin, PI_MCP2515_GPIO_FUNC_SPI);
	mcp2515_gpio_init(pi_mcp2515->cs_pin);

	/* TODO Sort conditional building with '1u' / GPIO_OUT */
	mcp2515_gpio_set_dir(pi_mcp2515->cs_pin, GPIO_OUT);

	pi_mcp2515->gpio_params = &gpio_params;
#elifdef USE_SPIDEV
	int spidev_fd;
	uint8_t mode, bits_per_word;

	mode = 0; /* TODO mode value */
	bits_per_word = 8;

	spidev_fd = open("", O_RDWR); /* TODO configure path */
	if (ioctl(spidev_fd, SPI_IOC_WR_MODE, &mode)
			|| ioctl(spidev_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word)
			|| ioctl(spidev_fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word)
			|| ioctl(spidev_fd, SPI_IOC_WR_MAX_SPEED_HZ, &pi_mcp2515->spi_clock)
			|| ioctl(spidev_fd, SPI_IOC_RD_MAX_SPEED_HZ, &pi_mcp2515->spi_clock)) {
		close(spidev_fd);
		/* TODO Error handle */
	}
	gpio_params = {
		.spidev_fd = spidev_fd,
	};
	pi_mcp2515->gpio_params = &gpio_params;
#elifdef USE_PRINT_DEBUG
	printf("spi_init(0x%02x, 0x%08x)\n", spi_channel, baudrate);
#endif
}

void
mcp2515_gpio_set_dir(uint8_t gpio, bool out)
{
#ifdef USE_PICO_LIB
	gpio_set_dir(gpio, out);
#elifdef USE_PRINT_DEBUG
	printf("gpio_set_dir(0x%02x, %s)\n", gpio, out ? "true" : "false");
#endif
}

void
mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	spi_write_blocking(spi_inst_from_index(pi_mcp2515->spi_channel), data, len);
#elifdef USE_SPIDEV
	struct spi_ioc_transfer tr = {};
	int res;

	res = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
#elifdef USE_PRINT_DEBUG
	printf("spi_write_blocking(0x%02x, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
		len == 1 ? "" : "...", len);
#endif
}

void
mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
#ifdef USE_PICO_LIB
	/* Note: For now, repeated_tx_data is not used anywhere in the library so we just skip it. */
	spi_read_blocking(spi_inst_from_index(pi_mcp2515->spi_channel), 0x00, data, len);
#elifdef USE_SPIDEV
	/* TODO */
#elifdef USE_PRINT_DEBUG
	printf("spi_read_blocking(0x%02x, 0x00, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
		len == 1 ? "" : "...", len);
#endif
}

void
mcp2515_gpio_put(uint8_t pin, uint8_t value)
{
#ifdef USE_PICO_LIB
	gpio_put(pin, value);
#elifdef USE_PRINT_DEBUG
	printf("gpio_put(0x%02x, 0x%02x)\n", pin, value);
#endif
}
