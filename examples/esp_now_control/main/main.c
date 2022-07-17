/* 
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
#include "nvs_flash.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_fs.h"

#include "esp_wifi.h"
#include "esp_netif.h"

#include "esp_utils.h"
#include "espnow.h"
#include "espnow_ctrl.h"

#include "ws2812.h"
#include "button.h"


static const char *TAG = "main";


typedef enum {
    ESPNOW_CTRL_INIT,
    ESPNOW_CTRL_BOUND,
    ESPNOW_CTRL_MAX
} espnow_ctrl_status_t;

static espnow_ctrl_status_t s_espnow_ctrl_status = ESPNOW_CTRL_INIT;

static void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());

    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void initiator_send_press_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_SINGLE_CLICK == button_get_event(arg)));
    ESP_LOGI(TAG, "initiator send press");
    static uint8_t status = 0;
    if (s_espnow_ctrl_status == ESPNOW_CTRL_BOUND) {
        espnow_ctrl_initiator_send(ESPNOW_ATTRIBUTE_KEY_1, ESPNOW_ATTRIBUTE_POWER, status);
        status ++;
        if (status > 2) status = 0;
    }
}

static void initiator_bind_press_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_DOUBLE_CLICK == button_get_event(arg)));
    ESP_LOGI(TAG, "initiator bind press");
    if (s_espnow_ctrl_status == ESPNOW_CTRL_INIT) {
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, true);
        s_espnow_ctrl_status = ESPNOW_CTRL_BOUND;
    }
}

static void initiator_unbind_press_cb(void *arg)
{
    ESP_ERROR_CHECK(!(BUTTON_LONG_PRESS_START == button_get_event(arg)));
    ESP_LOGI(TAG, "initiator unbind press");
    if (s_espnow_ctrl_status == ESPNOW_CTRL_BOUND) {
        espnow_ctrl_initiator_bind(ESPNOW_ATTRIBUTE_KEY_1, false);
        s_espnow_ctrl_status = ESPNOW_CTRL_INIT;
    }
}

static void app_driver_init(void)
{
    ESP_ERROR_CHECK(ws2812_init());

    button_gpio_config_t button_config = {
        .gpio_num = GPIO_KEY_BOOT,
        .active_level = 0,
    };
    button_handle_t button_handle = button_create(&button_config);
    button_register_cb(button_handle, BUTTON_SINGLE_CLICK, initiator_send_press_cb);
    button_register_cb(button_handle, BUTTON_DOUBLE_CLICK, initiator_bind_press_cb);
    button_register_cb(button_handle, BUTTON_LONG_PRESS_START, initiator_unbind_press_cb);
}

static void espnow_ctrl_responder_data_cb(espnow_attribute_t initiator_attribute,
                                     espnow_attribute_t responder_attribute,
                                     uint32_t status)
{
    ESP_LOGI(TAG, "espnow_ctrl_responder_recv, initiator_attribute: %d, responder_attribute: %d, value: %d",
                initiator_attribute, responder_attribute, status);

    switch (status)
    {
    case 0:
        ws2812_set_rgb(0, 255, 0, 0);
        ws2812_set_rgb(1, 255, 0, 0);
        ws2812_set_rgb(2, 255, 0, 0);
        ws2812_set_rgb(3, 255, 0, 0);
        ESP_ERROR_CHECK(ws2812_refresh());
        break;

    case 1:
        ws2812_set_rgb(0, 0, 255, 0);
        ws2812_set_rgb(1, 0, 255, 0);
        ws2812_set_rgb(2, 0, 255, 0);
        ws2812_set_rgb(3, 0, 255, 0);
        ESP_ERROR_CHECK(ws2812_refresh());
        break;

    case 2:
        ws2812_set_rgb(0, 0, 0, 255);
        ws2812_set_rgb(1, 0, 0, 255);
        ws2812_set_rgb(2, 0, 0, 255);
        ws2812_set_rgb(3, 0, 0, 255);
        ESP_ERROR_CHECK(ws2812_refresh());
        break;

    default:
        break;
    }
}

static void responder_light(void)
{
    app_driver_init();

    ESP_ERROR_CHECK(espnow_ctrl_responder_bind(30 * 1000, -55, NULL));
    espnow_ctrl_responder_data(espnow_ctrl_responder_data_cb);
}

static void espnow_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    if (base != ESP_EVENT_ESPNOW) {
        return;
    }

    switch (id) {
        case ESP_EVENT_ESPNOW_CTRL_BIND: {
            espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
            ESP_LOGI(TAG, "bind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);
            
            ws2812_set_rgb(0, 255, 255, 255);
            ws2812_set_rgb(1, 255, 255, 255);
            ws2812_set_rgb(2, 255, 255, 255);
            ws2812_set_rgb(3, 255, 255, 255);
            ESP_ERROR_CHECK(ws2812_refresh());
            break;
        }

        case ESP_EVENT_ESPNOW_CTRL_UNBIND: {
            espnow_ctrl_bind_info_t *info = (espnow_ctrl_bind_info_t *)event_data;
            ESP_LOGI(TAG, "unbind, uuid: " MACSTR ", initiator_type: %d", MAC2STR(info->mac), info->initiator_attribute);

            ws2812_set_rgb(0, 0, 0, 0);
            ws2812_set_rgb(1, 0, 0, 0);
            ws2812_set_rgb(2, 0, 0, 0);
            ws2812_set_rgb(3, 0, 0, 0);
            ESP_ERROR_CHECK(ws2812_refresh());
            break;
        }

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
    lv_label_set_text(label, "single-click: send \ndouble-click: bind \nlong-press: unbind ");             /*Set the labels text*/
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);

    esp_storage_init();

    wifi_init();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);

    esp_event_handler_register(ESP_EVENT_ESPNOW, ESP_EVENT_ANY_ID, espnow_event_handler, NULL);

    responder_light();

    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);
}


