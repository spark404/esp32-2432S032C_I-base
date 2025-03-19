/*
 * SPDX-FileCopyrightText: 2025 Hugo Trippaers
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_lvgl_port.h"
#include "lvgl.h"

extern lv_image_dsc_t openart_image_320x240;

void ui(lv_event_cb_t event_cb) {
    lv_obj_t *scr = lv_scr_act();

    /* Task lock */
    lvgl_port_lock(0);

    /* Create image */
    lv_obj_t *img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &openart_image_320x240);
    lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, 0, 0);

    /* Create a button */
    lv_obj_t *button = lv_button_create(scr);
    lv_obj_add_event_cb(button, event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(button, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lvgl_port_unlock();
}