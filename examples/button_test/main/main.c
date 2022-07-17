/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "bsp_i2c.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_fs.h"
#include "lv_demos.h"
#include "ws2812.h"
#include "button.h"

static const char *TAG = "main";


static void button_press_down_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_PRESS_DOWN == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_PRESS_DOWN");
}

static void button_press_up_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_PRESS_UP == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_PRESS_UP");
}

static void button_press_repeat_cb(void *arg)
{
    ESP_LOGI(TAG, "BUTTON_PRESS_REPEAT[%d]", button_get_repeat((button_handle_t)arg));
}

static void button_single_click_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_SINGLE_CLICK == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
}

static void button_double_click_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_DOUBLE_CLICK == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_DOUBLE_CLICK");
}

static void button_long_press_start_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_LONG_PRESS_START == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_LONG_PRESS_START");
}

static void button_long_press_hold_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_LONG_PRESS_HOLD == button_get_event(arg)));
    ESP_LOGI(TAG, "BUTTON_LONG_PRESS_HOLD");
}

static void print_button_event(button_handle_t btn)
{
    button_event_t evt = button_get_event(btn);
    switch (evt) {
    case BUTTON_PRESS_DOWN:
        ESP_LOGI(TAG, "BUTTON_PRESS_DOWN");
        break;
    case BUTTON_PRESS_UP:
        ESP_LOGI(TAG, "BUTTON_PRESS_UP");
        break;
    case BUTTON_PRESS_REPEAT:
        ESP_LOGI(TAG, "BUTTON_PRESS_REPEAT");
        break;
    case BUTTON_SINGLE_CLICK:
        ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
        break;
    case BUTTON_DOUBLE_CLICK:
        ESP_LOGI(TAG, "BUTTON_DOUBLE_CLICK");
        break;
    case BUTTON_LONG_PRESS_START:
        ESP_LOGI(TAG, "BUTTON_LONG_PRESS_START");
        break;
    case BUTTON_LONG_PRESS_HOLD:
        ESP_LOGI(TAG, "BUTTON_LONG_PRESS_HOLD");
        break;

    default:
        break;
    }
}

void lv_tick_task(void *arg)
{
    while(1)
    {
        vTaskDelay((10) / portTICK_PERIOD_MS);
        lv_task_handler();        
    }
}

void app_main(void)
{
    /* LVGL init */
    lv_init();
    lv_port_disp_init();
    lv_port_tick_init();

    lv_obj_t * label = lv_label_create(lv_scr_act());    /*Add a label to the button*/
    lv_label_set_text(label, "Try to single-click,\n double-click or \nlong-press the button");             /*Set the labels text*/
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);

    button_gpio_config_t button_config = {
        .gpio_num = GPIO_KEY_BOOT,
        .active_level = 0,
    };
    button_handle_t button_handle = button_create(&button_config);
    // button_register_cb(button_handle, BUTTON_PRESS_DOWN, button_press_down_cb);
    // button_register_cb(button_handle, BUTTON_PRESS_UP, button_press_up_cb);
    button_register_cb(button_handle, BUTTON_PRESS_REPEAT, button_press_repeat_cb);
    button_register_cb(button_handle, BUTTON_SINGLE_CLICK, button_single_click_cb);
    button_register_cb(button_handle, BUTTON_DOUBLE_CLICK, button_double_click_cb);
    button_register_cb(button_handle, BUTTON_LONG_PRESS_START, button_long_press_start_cb);
    button_register_cb(button_handle, BUTTON_LONG_PRESS_HOLD, button_long_press_hold_cb);

    ESP_LOGI(TAG, "init done");

    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);
}

