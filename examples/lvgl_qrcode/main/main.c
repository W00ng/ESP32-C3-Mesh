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

static const char *TAG = "main";


void ui_task(void *arg)
{
    static lv_obj_t *default_src;
    default_src = lv_scr_act();

    static char *str = "this is test qrcode!";
    lv_obj_t *qrcode = lv_qrcode_create(default_src, 150, lv_color_black(), lv_color_white());
    lv_qrcode_update(qrcode, str, strlen(str));
    lv_obj_align(qrcode, LV_ALIGN_CENTER, 0, 0); 
    // lv_obj_set_pos(qrcode, 160, 30);
    // vTaskDelay((3000) / portTICK_PERIOD_MS);
    // lv_qrcode_delete(qrcode);

    while(1) 
    {
        vTaskDelay((1000) / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
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

    ESP_LOGI(TAG, "init done");

    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);
    xTaskCreate(ui_task, "ui_task", 4096*4, NULL, 3, NULL);
}


