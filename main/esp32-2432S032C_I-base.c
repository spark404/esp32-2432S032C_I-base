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
#include <esp_lvgl_port.h>
#include <arch/sys_arch.h>
#include <driver/i2c_master.h>

#include "bsp.h"
#include "ui.h"

#define TAG "app_main"

#define ROTATE_FRAME   30

static lv_disp_t * disp_handle = NULL;
static lv_indev_t* touch_handle = NULL;

int toggle = 0;
void event_handler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        gpio_set_level(GPIO_LED_R, toggle);
        toggle = !toggle;
    }
}

void test_interrupt() {
    ESP_EARLY_LOGW(TAG, "test_interrupt");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Startup ESP32 2432S032C_I");
    ESP_ERROR_CHECK(backlight_init());
    ESP_ERROR_CHECK(backlight_set(0));

    const gpio_config_t led_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_LED_R | 1ULL << GPIO_LED_G | 1ULL << GPIO_LED_B
    };
    ESP_ERROR_CHECK(gpio_config(&led_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_R, 1));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_G, 1));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_B, 1));

    spi_bus_config_t spi_buscfg = {
        .sclk_io_num = GPIO_TFT_SPI_SCK,
        .mosi_io_num = GPIO_TFT_SPI_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PARALLEL_LINES * LCD_H_RES * 2 + 8
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &spi_buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t lcd_io_config = {
        .dc_gpio_num = GPIO_TFT_DC,
        .cs_gpio_num = GPIO_TFT_SPI_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &lcd_io_config, &lcd_io_handle));

    esp_lcd_panel_handle_t lcd_panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = GPIO_TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_dev_config, &lcd_panel_handle));


    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_panel_handle, false, true));


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
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 0,
            .mirror_y = 1,
        },
        .driver_data = &tp_gt911_config,
        .interrupt_callback = test_interrupt // Will be set by lvgl
    };

    esp_lcd_touch_handle_t tp = NULL;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp));

    // Initialize lvgl
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    // Add the screen
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io_handle,
        .panel_handle = lcd_panel_handle,
        .buffer_size = LCD_H_RES*40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
            .buff_spiram = false
        }
    };
    disp_handle = lvgl_port_add_disp(&disp_cfg);
    if (disp_handle == NULL) {
        ESP_LOGE(TAG, "Unable to setup display");
    }

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp_handle,
        .handle = tp,
    };
    touch_handle = lvgl_port_add_touch(&touch_cfg);
    if (touch_handle == NULL) {
        ESP_LOGE(TAG, "Unable to setup touchpad");
    }

    // Build the ui
    ui(event_handler);

    ESP_ERROR_CHECK(backlight_set(1));


    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
