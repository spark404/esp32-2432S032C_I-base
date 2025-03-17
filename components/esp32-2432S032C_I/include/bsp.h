/*
 * SPDX-FileCopyrightText: 2025 Hugo Trippaers
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BSP_H
#define BSP_H

#define GPIO_TFT_RST      12
#define GPIO_TFT_BL       27
#define GPIO_TFT_SPI_SCK  14
#define GPIO_TFT_SPI_MOSI 13
#define GPIO_TFT_SPI_CS   15
#define GPIO_TFT_DC        2

#define GPIO_TP_SCL       32
#define GPIO_TP_SDA       33
#define GPIO_TP_INT       12
#define GPIO_TP_RST       25

#define PARALLEL_LINES     1
#define LCD_H_RES        320
#define LCD_V_RES        240
#define LCD_HOST   SPI2_HOST
#define LCD_CMD_BITS       8
#define LCD_PARAM_BITS     8
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)  // This is a guess

#define GPIO_LED_R         4
#define GPIO_LED_G        16
#define GPIO_LED_B        17


esp_err_t backlight_init();
esp_err_t backlight_set(bool on);

#endif //BSP_H
