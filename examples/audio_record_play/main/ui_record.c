/**
 * @file ui_record.c
 * @brief Audio record example ui
 * @version 0.1
 * @date 2021-03-04
 * 
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 * 
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ui_record.h"

record_mode_t mode = idle;

#define COLOR_BAR   lv_color_make(86, 94, 102)
#define COLOR_THEME lv_color_make(252, 199, 0)
#define COLOR_DEEP  lv_color_make(246, 174, 61)
#define COLOR_TEXT  lv_color_make(56, 56, 56)
#define COLOR_BG    lv_color_make(238, 241, 245)


static void btn_cb(lv_event_t *event);


void ui_record(void)
{
    static lv_obj_t *default_src;
    default_src = lv_scr_act();		

    lv_obj_t *btn = lv_btn_create(default_src);
    lv_obj_set_size(btn, 100, 100);
    lv_obj_set_style_radius(btn, 50, LV_STATE_DEFAULT);
    lv_obj_set_pos(btn, 140-50, 100-50);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *label = lv_label_create(default_src);
    lv_label_set_text(label, "Press Button to Record & Play");
    lv_obj_set_style_text_font(label,   &lv_font_montserrat_18, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label,  lv_palette_main(LV_PALETTE_RED), LV_STATE_DEFAULT);
    lv_obj_align_to(label, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
}

static void btn_cb(lv_event_t *event)
{
    lv_obj_t *btn = (lv_obj_t *) event->target;

    if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
        // ESP_LOGI("main", "audio_record_start");
        mode = record;
    } else {
        // ESP_LOGI("main", "audio_play_start");
        mode = play;
    }
}


