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

LV_IMG_DECLARE(img1);
LV_IMG_DECLARE(img2);
LV_IMG_DECLARE(img3);
LV_IMG_DECLARE(img4);

void img_disp_task(void *arg)
{
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_obj_align(img, LV_ALIGN_CENTER, 0 , 0);

    while(1)
    {
        lv_img_set_src(img, &img1);
        vTaskDelay((3000) / portTICK_PERIOD_MS);
        lv_img_set_src(img, &img2);
        vTaskDelay((3000) / portTICK_PERIOD_MS);
        lv_img_set_src(img, &img3);
        vTaskDelay((3000) / portTICK_PERIOD_MS);
        lv_img_set_src(img, &img4);
        vTaskDelay((3000) / portTICK_PERIOD_MS);
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
    xTaskCreate(img_disp_task, "img_disp_task", 4096 * 2, NULL, 3, NULL);
}


