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

/* Notes to Help Navigating the `#ifdef` Labyrinth:
 *
 * USE_PICO_LIB / USE_SPI / USE_PRINT_DEBUG:
 *   - These three are set by CMake, or more specifically in the context of this file, by `-D` arguments to the compiler
 *     for the purposes of conditional building. USE_SPI covers Linux or BSD systems, while `USE_PICO_LIB` will cover
 *     using the Raspberry Pi Pico SDK. `USE_PRINT_DEBUG` is for testing purposes with very limited practical use. It
 *     turns all functionality into a NOOP and prints function arguments to stdout.
 *
 * USE_SPIDEV_LINUX:
 *   - This is set automatically when built with `USE_SPI` and on a Linux system. It covers conditions for handling SPI
 *     interactions on Linux.
 *
 * USE_SPIGEN_BSD:
 *   - This is set automatically when built with `USE_SPI` and on a BSD system to use spigen for (which is only FreeBSD,
 *     at least at present). It covers conditions for using spigen to handle SPI interactions.
 *
 * USE_SPI_BSD:
 *   - This is set automatically when built with `USE_SPI` and on a BSD system not using spigen, which is only NetBSD
 *     for the time being as OpenBSD may be added under this at some point if found to be possible. It covers conditions
 *     for handling SPI interactions on BSD without spigen.
 *
 * USE_BSD_GPIO:
 *   - This is set automatically when built with `USE_SPI` on all supported BSD operating systems. Currently, all the
 *     GPIO interactions via IOCTL are mostly identical on BSDs minus a couple tiny quirks noted later in comments.
 *   - When `USE_BSD_GPIO` gets defined, `gpio_set_t` is also defined to be `struct gpio_pin` on FreeBSD, and
 *     `struct gpio_set` on other BSDs. The contents of both structs are the exact same, so we just typedef it and use
 *     the typedef in this file so the same code can be used in either case.
 *   - The FreeBSD ioctls use `GPIOSET`, whereas the others use `GPIOWRITE` for the same purpose. On FreeBSD, we define
 *     `GPIOWRITE` to be the same as `GPIOSET` such that we can use `GPIOWRITE` in this file even on FreeBSD.
 *   - This should be used when referring to BSD GPIO logic. If some code supports all BSDs but is only related to SPI
 *     and not GPIO, then the `#ifdef` should be `defined(USE_SPI_BSD) || defined(USE_SPIGEN_BSD)` and not
 *     `USE_BSD_GPIO`.
 *
 *
 * Code Style and Cleanliness:
 *
 * This file is awful to keep other files clean. The `#ifdef`s make it hard to keep good consistent style, but generally
 * the idea is to make the best effort possible to keep to OpenBSD's style(9) guidelines through the chaos. Just use
 * approach it logically. While this nightmare beast of a file is generally doomed to be annoying, all efforts will still
 * be made to get this as clean as possible with future refactoring, and with the quality of new code.
 */

#ifdef USE_PICO_LIB

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#elif defined(USE_SPI)

#include <sys/stat.h>
#include <stdio.h>

#if defined(__linux__)
#define USE_SPIDEV_LINUX
#elif defined(__NetBSD__)
/* OpenBSD seems extremely limited in its SPI support. For now, it is excluded. */
#define USE_SPI_BSD
#define USE_BSD_GPIO
#elif defined(__FreeBSD__)
#define USE_SPIGEN_BSD
#define USE_BSD_GPIO
#else
#error "Unsupported OS"
#endif

#include <sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>

#elif defined(USE_PRINT_DEBUG)
#include <stdio.h>
#endif

#ifdef USE_SPIDEV_LINUX
#include <linux/gpio.h>
#include <linux/spi/spidev.h>
#elif defined(USE_SPI_BSD)
/* <dev/spi/spi_io.h> and <sys/spigenio.h> are both absent on OpenBSD. <dev/spi/spivar.h> exists though. */
#include <dev/spi/spi_io.h>
#include <sys/gpio.h>
typedef struct gpio_set gpio_set_t;
#elif  defined(USE_SPIGEN_BSD)
#include <sys/spigenio.h>
#include <sys/gpio.h>
#define GPIOWRITE GPIOSET
typedef struct gpio_pin gpio_set_t;
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pi_MCP2515_handle.h"

#include "gpio.h"

#ifdef USE_SPI
static int	spi_duplex_com(const pi_mcp2515_t *, char[sizeof(uint64_t)], size_t, char[sizeof(uint64_t)]);

/**
 * @brief Perform a round of full duplex communication over SPI.
 *
 * spidev only does duplex communication. So, we split that off to this function.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param tx_buffer the buffer to use for transmitting.
 * @param tx_len the length of the tx data.
 * @param rx_buffer the buffer to use for receiving.
 * @return zero if success, otherwise non-zero.
 */
static int
spi_duplex_com(const pi_mcp2515_t *pi_mcp2515, char tx_buffer[sizeof(uint64_t)], /* NOLINT(*-non-const-parameter) */
    size_t tx_len, char rx_buffer[sizeof(uint64_t)]) /* NOLINT(*-non-const-parameter) */
{
	int res = 0;

	if (tx_len > sizeof(uint64_t)) {
		res = -1;
		goto err;
	}
#if defined(USE_SPIDEV_LINUX)
	struct spi_ioc_transfer tr = {
		.tx_buf = *(uint64_t *)tx_buffer,
		.rx_buf = *(uint64_t *)rx_buffer,
		.len = sizeof(uint64_t),
		.delay_usecs = pi_mcp2515->gpio_spi_delay_usec,
		.speed_hz = pi_mcp2515->spi_clock,
		.bits_per_word = pi_mcp2515->gpio_spi_bits_per_word,
	};

	res = ioctl(pi_mcp2515->gpio_spidev_fd, SPI_IOC_MESSAGE(1), &tr);
#elif defined(USE_SPI_BSD)
	spi_ioctl_transfer_t tr = {
		.sit_send = tx_buffer,
		.sit_sendlen = tx_len,
		.sit_recv = rx_buffer,
		.sit_recvlen = sizeof(uint64_t),
		/* .sit_addr ? */
	};

	res = ioctl(pi_mcp2515->gpio_spidev_fd, SPI_IOCTL_TRANSFER, &tr);
#elif defined(USE_SPIGEN_BSD)
	struct spigen_transfer transfer =  { 0 };

	transfer.st_command.iov_base = tx_buffer;
	transfer.st_command.iov_len = tx_len;
	transfer.st_data.iov_base = rx_buffer;
	transfer.st_data.iov_len = sizeof(uint64_t);

	res = ioctl(pi_mcp2515->gpio_spidev_fd, SPIGENIOC_TRANSFER, &transfer);
#endif

err:
	return (res);
}


static char *spi_dev_defaults[] = {
	"/dev/spiN",
	"/dev/spi0.N"
#ifdef USE_SPIGEN_BSD
	"/dev/spigenN" /* Only bother checking this if using spigen */
#endif
};

static char *gpio_dev_defaults[] = {
	"/dev/gpio0",
	"/dev/gpiochip0"
};


static char	*find_spi_dev_path(uint8_t);
static char	*find_gpio_dev_path(void);


/**
 * @brief Find a spi device in /dev.
 *
 * Note: This will modify the defaults map. This doesn't matter, however, because we only change the last character so
 * we can easily just overwrite on subsequent runs, and there isn't a reason for subsequent runs after init anyway.
 *
 * @param spi_channel the spi channel.
 * @return the path found or NULL if not found.
 */
static char *
find_spi_dev_path(const uint8_t spi_channel)
{
	struct stat s;
	size_t i;
	char *cur_path = NULL, *res = NULL;

	for (i = 0; i < sizeof(spi_dev_defaults) / sizeof(spi_dev_defaults[0]); i++) {
		cur_path = spi_dev_defaults[i];
		cur_path[strlen(cur_path) - 1] = spi_channel == 0 ? '0' : '1';
		if (stat(cur_path, &s) != 0) {
			res = cur_path;
			goto end;
		}
	}

end:
	return (res);
}


/**
 * @brief Find a gpio device in /dev.
 *
 * @return the path found or NULL if not found.
 */
static char *
find_gpio_dev_path(void)
{
	struct stat s;
	size_t i;
	char *cur_path = NULL, *res = NULL;

	for (i = 0; i < sizeof(gpio_dev_defaults) / sizeof(gpio_dev_defaults[0]); i++) {
		cur_path = gpio_dev_defaults[i];
		if (stat(cur_path, &s) != 0) {
			res = cur_path;
			goto end;
		}
	}

end:
	return (res);
}
#endif /* USE_SPI */


/**
 * @brief Set up a GPIO pin.
 *
 * When compiled for Pico, it will simply call `gpio_init(pin)`.
 *
 * With spidev, this will open a file descriptor for the specified pin to prepare it for future use.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param pin the GPIO pin to set up.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_gpio_init(pi_mcp2515_t *pi_mcp2515, uint8_t pin)
{
	int res = 0;
#ifdef USE_PICO_LIB
	gpio_init(pin);
#elif defined(USE_SPIDEV_LINUX)
	struct gpio_v2_line_request rq = { 0 };

	rq.offsets[0] = pin;
	rq.num_lines = 1;
	rq.config.flags = GPIOHANDLE_REQUEST_INPUT;
	rq.config.num_attrs = 0;
	strncpy(rq.consumer, "pi_mcp2515", sizeof(rq.consumer));

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIO_GET_LINEHANDLE_IOCTL, &rq);
	if (!res)
		pi_mcp2515->gpio_pin_fd_map[pin] = rq.fd;
#elif defined(USE_BSD_GPIO)
	gpio_set_t pin_config = { 0 };

	snprintf(pin_config.gp_name, GPIOMAXNAME, "pi_mcp2515 pin #%d", pin);
	pin_config.gp_flags = GPIO_PIN_INPUT;

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIOSET, &pin_config);
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_init(0x%02x)\n", pin);
#endif
	return (res);
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
#if defined(USE_SPIDEV_LINUX) || defined(USE_SPI_BSD) || defined(USE_SPIGEN_BSD)
	if (pi_mcp2515->gpio_spidev_fd > 0)
		close(pi_mcp2515->gpio_spidev_fd);

	if (pi_mcp2515->gpio_gpio_fd > 0)
		close(pi_mcp2515->gpio_gpio_fd);

#ifdef USE_SPIDEV_LINUX
	for (uint8_t i = 0; i < PI_MCP2515_GPIO_PIN_MAP_LEN; i++)
		if (pi_mcp2515->gpio_pin_fd_map[i] > 0)
			close(pi_mcp2515->gpio_pin_fd_map[i]);
#endif /* USE_SPIDEV_LINUX */
#endif
}


/**
 * @brief Init SPI functionality with default options.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_gpio_spi_init(pi_mcp2515_t *pi_mcp2515)
{
	return (mcp2515_gpio_spi_init_full_optional(pi_mcp2515, 0, 8));
}


/**
 * @brief Init SPI functionality with additional options.
 *
 * @param pi_mcp2515 the piMCP2515 handle.
 * @param mode SPI mode.
 * @param bits_per_word SPI bits per word.
 * @return zero if success, otherwise non-zero.
 */
int
mcp2515_gpio_spi_init_full_optional(pi_mcp2515_t *pi_mcp2515, uint8_t mode, uint8_t bits_per_word)
{
	int res = 0;
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
		res = -1;
		goto err;
	}

	pi_mcp2515->gpio_spi_inst = spi_inst,

	spi_init(spi_inst, pi_mcp2515->spi_clock);
	gpio_set_function(pi_mcp2515->tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->rx_pin, GPIO_FUNC_SPI);
	gpio_set_function(pi_mcp2515->sck_pin, GPIO_FUNC_SPI);
	mcp2515_gpio_init(pi_mcp2515, pi_mcp2515->cs_pin);

	mcp2515_gpio_set_dir(pi_mcp2515, pi_mcp2515->cs_pin, true);
#elif defined(USE_SPI)
#ifdef USE_SPI_BSD
	spi_ioctl_configure_t spi_cfg = { 0 };
#endif
	int spidev_fd, gpio_fd;
	char *spidev_path, *gpiodev_path;

	if (pi_mcp2515->gpio_dev_spi_path == NULL) {
		spidev_path = find_spi_dev_path(pi_mcp2515->spi_channel);
	} else
		spidev_path = pi_mcp2515->gpio_dev_spi_path;
	if (spidev_path == NULL) {
		res = -1;
		goto err;
	}
	if (pi_mcp2515->gpio_dev_gpio_path == NULL) {
		gpiodev_path = find_gpio_dev_path();
	} else
		gpiodev_path = pi_mcp2515->gpio_dev_gpio_path;
	if (gpiodev_path == NULL) {
		res = -1;
		goto err;
	}

	gpio_fd = open(gpiodev_path, O_RDWR);
	if (gpio_fd < 0) {
		res = -1;
		goto err;
	}
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

	pi_mcp2515->gpio_spi_bits_per_word = bits_per_word;
	pi_mcp2515->gpio_spi_delay_usec = 0;
#elif defined(USE_SPI_BSD)
	spi_cfg.sic_mode = pi_mcp2515->gpio_spi_mode;
	spi_cfg.sic_speed = pi_mcp2515->spi_clock; /* NOLINT(*-narrowing-conversions) */

	if ((res = ioctl(spidev_fd, SPI_IOCTL_CONFIGURE, &spi_cfg))) {
		close(gpio_fd);
		close(spidev_fd);
		goto err;
	}
#elif defined(USE_SPIGEN_BSD)
	if ((res = ioctl(spidev_fd, SPIGENIOC_SET_CLOCK_SPEED))) {
		close(gpio_fd);
		close(spidev_fd);
		goto err;
	}
	if ((res = ioctl(spidev_fd, SPIGENIOC_SET_SPI_MODE))) {
		close(gpio_fd);
		close(spidev_fd);
		goto err;
	}
#endif

	pi_mcp2515->gpio_gpio_fd = gpio_fd;
	pi_mcp2515->gpio_spidev_fd = spidev_fd;
	pi_mcp2515->gpio_spi_mode = mode;
#elif defined(USE_PRINT_DEBUG)
	printf("spi_init(0x%02x, 0x%08x)\n", pi_mcp2515->spi_channel, pi_mcp2515->spi_clock);
#endif

err:
	return (res);
}


int
mcp2515_gpio_set_dir(const pi_mcp2515_t *pi_mcp2515, uint8_t gpio, bool out)
{
	int res = 0;

#ifdef USE_PICO_LIB
	gpio_set_dir(gpio, out);
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_set_dir(0x%02x, %s)\n", gpio, out ? "true" : "false");
#elif defined(USE_SPIDEV_LINUX)
	struct gpio_v2_line_config config = { 0 };

	if (out)
		config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
	else
		config.flags = GPIO_V2_LINE_FLAG_INPUT;

	res = ioctl(pi_mcp2515->gpio_pin_fd_map[gpio], GPIO_V2_LINE_SET_CONFIG_IOCTL, &config);
#elif defined(USE_BSD_GPIO)
	/* TODO Confirm behaviour with second GPIOSET on things like name. */
	gpio_set_t pin_config = { 0 };

	pin_config.gp_pin = gpio;
	if (out)
		pin_config.gp_flags = GPIO_PIN_OUTPUT;
	else
		pin_config.gp_flags = GPIO_PIN_INPUT;

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIOSET, &pin_config);
#endif
	return (res);
}


int
mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
	int res = 0;

#ifdef USE_PICO_LIB
	spi_write_blocking(pi_mcp2515->gpio_spi_inst, data, len);
#elif defined(USE_SPI)
	/* TODO incomplete */
	size_t chunk_len;
	int tail_len;
	char tx_buffer[sizeof(uint64_t)] = { 0 }, rx_buffer[sizeof(tx_buffer)] = { 0 };

	for (uint8_t i = 0; i < len; i += sizeof(uint64_t)) {
		if (len - i > sizeof(uint64_t)) {
			memcpy(&tx_buffer, &data[i], sizeof(tx_buffer));
			chunk_len = sizeof(uint64_t);
		} else {
			tail_len = len - i;
			memcpy(&tx_buffer, &data[i], tail_len);
			memset(&tx_buffer[tail_len], 0, sizeof(tx_buffer) - tail_len);
			chunk_len = sizeof(tx_buffer) - tail_len;
		}

		if ((res = spi_duplex_com(pi_mcp2515, tx_buffer, chunk_len, rx_buffer)))
			goto err;
	}
#elif defined(USE_PRINT_DEBUG)
	printf("spi_write_blocking(0x%02x, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
	    len == 1 ? "" : "...", len);
#endif

err:
	return (res);
}

int
mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *pi_mcp2515, uint8_t *data, uint8_t len)
{
	int res = 0;
#ifdef USE_PICO_LIB
	/* Note: For now, repeated_tx_data is not used anywhere in the library so we just skip it. */
	spi_read_blocking(pi_mcp2515->gpio_spi_inst, 0x00, data, len);
#elif defined(USE_SPI)
	/* TODO incomplete */
	char tx_buffer[sizeof(uint64_t)] = { 0 }, rx_buffer[sizeof(tx_buffer)] = { 0 };

	for (uint8_t i = 0; i < len; i += sizeof(uint64_t)) {
		if ((res = spi_duplex_com(pi_mcp2515, tx_buffer, 0, rx_buffer)))
			goto err;
		memcpy(&data[i], &rx_buffer, len - i > sizeof(uint64_t) ? sizeof(rx_buffer) : len - i);
	}
#elif defined(USE_PRINT_DEBUG)
	printf("spi_read_blocking(0x%02x, 0x00, 0x%02x%s, 0x%02x)\n", pi_mcp2515->spi_channel, data[0],
	    len == 1 ? "" : "...", len);
#endif

err:
	return (res);
}


int
mcp2515_gpio_put(const pi_mcp2515_t *pi_mcp2515, uint8_t pin, uint8_t value)
{
	int res = 0;
#ifdef USE_PICO_LIB
	gpio_put(pin, value);
#elif defined(USE_SPIDEV_LINUX)
	struct gpio_v2_line_values values;

	values.mask = (1 << (pin - 1));
	values.bits = value ? 1 : 0;

	res = ioctl(pi_mcp2515->gpio_pin_fd_map[pin], GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
#elif defined(USE_BSD_GPIO)
	struct gpio_req pin_op;

	pin_op.gp_pin = pin;
	pin_op.gp_value = value ? 1 : 0;

	res = ioctl(pi_mcp2515->gpio_gpio_fd, GPIOWRITE, &pin_op);
#elif defined(USE_PRINT_DEBUG)
	printf("gpio_put(0x%02x, 0x%02x)\n", pin, value);
#endif
	return (res);
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
