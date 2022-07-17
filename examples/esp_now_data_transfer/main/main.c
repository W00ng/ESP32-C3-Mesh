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


#if  1   // button interrupt configure
static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_interrupt_task(void* arg)
{
    uint32_t io_num;
    esp_err_t ret  = ESP_OK;
    uint32_t count = 0;
    size_t size    = 0;
    uint8_t *data  = ESP_CALLOC(1, ESPNOW_DATA_LEN);

    espnow_frame_head_t frame_head = {
        .retransmit_count = 5,         // CONFIG_RETRY_NUM
        .broadcast        = true,
    };

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))  {
            // printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if ((GPIO_KEY_BOOT == io_num) && (0 == gpio_get_level(GPIO_KEY_BOOT)))
            {
                // ESP_LOGI(TAG, "wake button pressed!");
                sprintf((char*)data, "esp-now data transfer %d", count++);
                size = strlen((char*)data);

                ret = espnow_send(ESPNOW_TYPE_DATA, ESPNOW_ADDR_BROADCAST, data, size, &frame_head, portMAX_DELAY);
                ESP_ERROR_CONTINUE(ret != ESP_OK, "<%s> espnow_send", esp_err_to_name(ret));

                ESP_LOGI(TAG, "data=%s  len=%d", data, size);
                memset(data, 0, ESPNOW_DATA_LEN);
            }
        }
    }
}

static void gpio_interrupt_config(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        //.intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << GPIO_KEY_BOOT,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    gpio_set_intr_type(GPIO_KEY_BOOT, GPIO_INTR_NEGEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    xTaskCreate(gpio_interrupt_task, "gpio_interrupt_task", 4 * 1024, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_KEY_BOOT, gpio_isr_handler, (void*) GPIO_KEY_BOOT);
}
#endif

static void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static esp_err_t receive_handle(uint8_t *src_addr, void *data,
                      size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);
    ESP_PARAM_CHECK(size);
    ESP_PARAM_CHECK(rx_ctrl);

    static uint32_t count = 0;

    ESP_LOGI(TAG, "espnow_recv, <%d> [" MACSTR "][%d][%d][%d]: %.*s", 
            count++, MAC2STR(src_addr), rx_ctrl->channel, rx_ctrl->rssi, size, size, (char *)data);

    return ESP_OK;
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

    lv_obj_t * label = lv_label_create(lv_scr_act());       /*Add a label to the button*/
    lv_label_set_text(label, "click button to send");       /*Set the labels text*/
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label);

    /* button init */
    gpio_interrupt_config();

    esp_storage_init();
    esp_event_loop_create_default();

    wifi_init();

    espnow_config_t espnow_config = ESPNOW_INIT_CONFIG_DEFAULT();
    espnow_init(&espnow_config);

    espnow_set_type(ESPNOW_TYPE_DATA, 1, receive_handle);

    xTaskCreate(lv_tick_task, "lv_tick_task", 4 * 1024, NULL, 1, NULL); 
}


