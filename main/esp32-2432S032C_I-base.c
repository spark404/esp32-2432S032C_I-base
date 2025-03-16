/*
 * SPDX-FileCopyrightText: 2025 Hugo Trippaers
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp_lcd_io_i2c.h>
#include <esp_lcd_io_spi.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_st7789.h>
#include <esp_lcd_touch_gt911.h>
#include <stdio.h>
#include <esp_log.h>
#include <arch/sys_arch.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>

#include "bsp.h"
#include "pretty_effect.h"
#include "../../../SDKs/esp-idf/components/esp_driver_i2c/i2c_private.h"

#define TAG "app_main"

#define ROTATE_FRAME   30

extern uint8_t openart_image_320x240_map[];

static uint16_t *s_lines[2];
static void display_pretty_colors(esp_lcd_panel_handle_t panel_handle)
{
    int frame = 0;
    // Indexes of the line currently being sent to the LCD and the line we're calculating
    int sending_line = 0;
    int calc_line = 0;

    // After ROTATE_FRAME frames, the image will be rotated
    while (frame <= ROTATE_FRAME) {
        frame++;
        for (int y = 0; y < LCD_V_RES; y += PARALLEL_LINES) {
            // Calculate a line
            pretty_effect_calc_lines(s_lines[calc_line], y, frame, PARALLEL_LINES);
            sending_line = calc_line;
            calc_line = !calc_line;
            // Send the calculated data
            esp_lcd_panel_draw_bitmap(panel_handle, 0, y, 0 + LCD_H_RES, y + PARALLEL_LINES, s_lines[sending_line]);
        }
    }
}

uint16_t touch_x[1];
uint16_t touch_y[1];
uint16_t touch_strength[1];
uint8_t touch_cnt = 0;
void tp_interrupt(esp_lcd_touch_handle_t tp) {
    ESP_LOGI(TAG, "touch interrupt");
    if  (esp_lcd_touch_get_coordinates(tp, touch_x, touch_y, touch_strength, &touch_cnt, 1)) {
        ESP_LOGI(TAG, "touch coordinates: %d, %d", touch_x[0], touch_y[0]);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Startup ESP32 2432S032C_I");
    ESP_ERROR_CHECK(backlight_init());
    ESP_ERROR_CHECK(backlight_set(0));

    spi_bus_config_t buscfg = {
        .sclk_io_num = GPIO_TFT_SPI_SCK,
        .mosi_io_num = GPIO_TFT_SPI_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * LCD_H_RES * 2 + 8
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = GPIO_TFT_DC,
        .cs_gpio_num = GPIO_TFT_SPI_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = GPIO_TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_dev_config, &panel_handle));


    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));

    ESP_ERROR_CHECK(backlight_set(1));

    i2c_master_bus_handle_t tp_bus_handle = NULL;
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = GPIO_TP_SCL,
        .sda_io_num = GPIO_TP_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &tp_bus_handle));
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = 100000;

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(tp_bus_handle, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = tp_io_config.dev_addr,
    };

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_V_RES,
        .y_max = LCD_H_RES,
        .rst_gpio_num = GPIO_TP_RST,
        .int_gpio_num = GPIO_TP_INT,
        .levels = {
            .reset = 0,
            .interrupt = 1,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 1,
        },
        .driver_data = &tp_gt911_config,
        .interrupt_callback = tp_interrupt
    };

    esp_lcd_touch_handle_t tp = NULL;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp));

    ESP_ERROR_CHECK(pretty_effect_init());

    // Allocate memory for the pixel buffers
    for (int i = 0; i < 2; i++) {
        s_lines[i] = heap_caps_malloc(LCD_H_RES * PARALLEL_LINES * sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(s_lines[i] != NULL);
    }

    // Start and rotate
    while (1) {
        // Display
        display_pretty_colors(panel_handle);
    }
}
