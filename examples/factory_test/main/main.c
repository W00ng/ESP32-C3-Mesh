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
#include "bsp_lcd.h"
#include "es8311.h"
#include "bat_adc.h"
#include "htu21.h"
#include "ws2812.h"

static const char *TAG = "main";

#if 1
static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_interrupt_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if ((GPIO_KEY_BOOT == io_num) && (0 == gpio_get_level(GPIO_KEY_BOOT)))
            {
                ESP_LOGI(TAG, "wake button pressed!");
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

    xTaskCreate(gpio_interrupt_task, "gpio_interrupt_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_KEY_BOOT, gpio_isr_handler, (void*) GPIO_KEY_BOOT);
}
#endif

void ws2812_task(void *arg)
{
    while(1) 
    {
        ws2812_set_rgb(0, 255, 0, 0);
        ws2812_set_rgb(1, 255, 0, 0);
        ws2812_set_rgb(2, 255, 0, 0);
        ws2812_set_rgb(3, 255, 0, 0);
        ESP_ERROR_CHECK(ws2812_refresh());
        vTaskDelay(1000/portTICK_PERIOD_MS);
        ws2812_set_rgb(0, 0, 255, 0);
        ws2812_set_rgb(1, 0, 255, 0);
        ws2812_set_rgb(2, 0, 255, 0);
        ws2812_set_rgb(3, 0, 255, 0);
        ESP_ERROR_CHECK(ws2812_refresh());
        vTaskDelay(1000/portTICK_PERIOD_MS);    
        ws2812_set_rgb(0, 0, 0, 255);
        ws2812_set_rgb(1, 0, 0, 255);
        ws2812_set_rgb(2, 0, 0, 255);
        ws2812_set_rgb(3, 0, 0, 255);
        ESP_ERROR_CHECK(ws2812_refresh());
        vTaskDelay(1000/portTICK_PERIOD_MS);     
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    /* Initialize I2C 400KHz */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));

    /* LCD init */
    ESP_ERROR_CHECK(bsp_lcd_init());

    /* Init WS2812 */
    ESP_ERROR_CHECK(ws2812_init());

    ESP_ERROR_CHECK(es8311_codec_config(AUDIO_HAL_44K_SAMPLES));
    es8311_read_chipid();

    gpio_interrupt_config();

    ESP_LOGI(TAG, "init done");

    xTaskCreate(ws2812_task, "ws2812_task", 4096, NULL, 2, NULL);
}


