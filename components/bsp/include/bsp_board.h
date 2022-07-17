
#ifndef _BSP_BOARD_H_
#define _BSP_BOARD_H_

#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"


/**
 * @brief ESP32-C3 LCD GPIO defination
 * 
 */
#define LCD_WIDTH           (280)
#define LCD_HEIGHT          (240)
#define LCD_CMD_BITS        (8)
#define LCD_PARAM_BITS      (8)
#define LCD_SPI_CLOCK_HZ    (40 * 1000 * 1000)
#define LCD_SPI_HOST        (SPI2_HOST)

#define GPIO_LCD_SCK     (GPIO_NUM_7)
#define GPIO_LCD_CS      (GPIO_NUM_21)
#define GPIO_LCD_SDA     (GPIO_NUM_10)
#define GPIO_LCD_DC      (GPIO_NUM_20)

/**
 * @brief ESP32-C3 I2C GPIO defination
 * 
 */
#define GPIO_I2C_SCL     (GPIO_NUM_8)
#define GPIO_I2C_SDA     (GPIO_NUM_2)

/**
 * @brief ESP32-C3 I2S GPIO defination
 * 
 */
#define GPIO_I2S_MCLK    (GPIO_NUM_0)
#define GPIO_I2S_SCLK    (GPIO_NUM_6)
#define GPIO_I2S_LRCK    (GPIO_NUM_4)
#define GPIO_I2S_DIN     (GPIO_NUM_5)
#define GPIO_I2S_DOUT    (GPIO_NUM_1)

/**
 * @brief ESP32-C3 others GPIO defination
 * 
 */
#define GPIO_KEY_BOOT    (GPIO_NUM_9)

// ADC and RMT can't use at the same time
#define GPIO_BAT_ADC     (GPIO_NUM_3)
#define GPIO_RMT_SIG     (GPIO_NUM_3)
#define RGB_LED_NUM      (4)

#endif

