/*
 * SPDX-FileCopyrightText: 2025 Hugo Trippaers
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_err.h>
#include <driver/gpio.h>

#include "bsp.h"

/// @brief Configure the backlight of the LCD panel
esp_err_t backlight_init() {
    const gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_TFT_BL
    };
    return gpio_config(&bk_gpio_config);
}

/// @brief Configure the backlight of the LCD panel
/// @param on Enable the backlight when true
esp_err_t backlight_set(const bool on) {
    return gpio_set_level(GPIO_TFT_BL, on);
}