/*
 * Created by Roos Catling-Tate.
 * 
 * Copyright 2025
 */

#ifndef GPIO_H
#define GPIO_H

/* This file is for conditionally selecting which backend SPI/GPIO functionality is used. For instance, the Pi Pico SDK
 * or using print debugging so that logic can be tested locally.
 */

#ifdef USE_PICO_LIB
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#endif

#define SET_CS(x) mcp2515_gpio_put(x->cs_pin, 1)
#define UNSET_CS(x) mcp2515_gpio_put(x->cs_pin, 0)

#define PI_MCP2515_GPIO_FUNC_XIP 0
#define PI_MCP2515_GPIO_FUNC_SPI 1
#define PI_MCP2515_GPIO_FUNC_UART 2
#define PI_MCP2515_GPIO_FUNC_I2C 3
#define PI_MCP2515_GPIO_FUNC_PWM 4
#define PI_MCP2515_GPIO_FUNC_SIO 5
#define PI_MCP2515_GPIO_FUNC_PIO0 6
#define PI_MCP2515_GPIO_FUNC_PIO1 7
#define PI_MCP2515_GPIO_FUNC_GPCK 8
#define PI_MCP2515_GPIO_FUNC_USB 9
#define PI_MCP2515_GPIO_FUNC_NULL 0x1f
#include "pi_MCP2515.h"

void	mcp2515_gpio_function_set(uint8_t, uint8_t);
void	mcp2515_gpio_init(uint8_t);
void	mcp2515_gpio_set_dir(uint8_t gpio, bool out);
void	mcp2515_gpio_spi_init(uint8_t, uint32_t);
void	mcp2515_gpio_spi_write_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
void	mcp2515_gpio_spi_read_blocking(pi_mcp2515_t *, uint8_t[], uint8_t);
void	mcp2515_gpio_put(uint8_t, uint8_t);

#endif /* GPIO_H */
