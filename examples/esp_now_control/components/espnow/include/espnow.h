// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "esp_now.h"
#include "esp_utils.h"
#include "espnow_security.h"

#ifdef __cplusplus
extern "C" {
#endif /**< _cplusplus */

#define ESPNOW_PAYLOAD_LEN                  (230)
#ifdef CONFIG_APP_SECURITY
#define ESPNOW_DATA_LEN                     ESPNOW_SEC_PACKET_MAX_SIZE
#else
#define ESPNOW_DATA_LEN                     ESPNOW_PAYLOAD_LEN
#endif
#define ESPNOW_ADDR_LEN                     (6)
#define ESPNOW_DECLARE_COMMON_ADDR(addr)    extern const uint8_t addr[6];
#define ESPNOW_ADDR_IS_EMPTY(addr)          (((addr)[0] | (addr)[1] | (addr)[2] | (addr)[3] | (addr)[4] | (addr)[5]) == 0x0)
#define ESPNOW_ADDR_IS_BROADCAST(addr)      (((addr)[0] & (addr)[1] & (addr)[2] & (addr)[3] & (addr)[4] & (addr)[5]) == 0xFF)
#define ESPNOW_ADDR_IS_SELF(addr)           !memcmp(addr, ESPNOW_ADDR_SELF, 6)
#define ESPNOW_ADDR_IS_EQUAL(addr1, addr2)  !memcmp(addr1, addr2, 6)

typedef uint8_t (espnow_addr_t)[6];
typedef uint8_t (espnow_group_t)[6];

ESPNOW_DECLARE_COMMON_ADDR(ESPNOW_ADDR_NONE);
ESPNOW_DECLARE_COMMON_ADDR(ESPNOW_ADDR_BROADCAST);
ESPNOW_DECLARE_COMMON_ADDR(ESPNOW_ADDR_GROUP_OTA);
ESPNOW_DECLARE_COMMON_ADDR(ESPNOW_ADDR_GROUP_SEC);
ESPNOW_DECLARE_COMMON_ADDR(ESPNOW_ADDR_GROUP_PROV);

/**
 * @brief Declaration of the task events family
 */
ESP_EVENT_DECLARE_BASE(ESP_EVENT_ESPNOW);
#define ESP_EVENT_ESPNOW_PROV_BASE          0x100
#define ESP_EVENT_ESPNOW_CTRL_BASE          0x200
#define ESP_EVENT_ESPNOW_OTA_BASE           0x300
#define ESP_EVENT_ESPNOW_DEBUG_BASE         0x400
#define ESP_EVENT_ESPNOW_RESERVED_BASE      0x500

/**
 * @brief The channel on which the device sends packets
 */
#define ESPNOW_CHANNEL_CURRENT               0x0   /**< Only in the current channel */
#define ESPNOW_CHANNEL_ALL                   0x0f  /**< All supported channels */

#define ESPNOW_RETRANSMIT_MAX_COUNT          0x1f  /**< Maximum number of retransmissions */
#define ESPNOW_FORWARD_MAX_COUNT             0x1f  /**< Maximum number of forwards */

/**
 * @brief Initialize the configuration of espnow
 */
typedef struct {
    const uint8_t pmk[16];       /**< Primary master key */
    bool forward_enable;         /**< Forward when packets are received */
    bool forward_switch_channel; /**< Forward data packet with exchange channel */
    uint8_t send_retry_num;      /**< Number of retransmissions */
    uint32_t send_max_timeout;   /**< Maximum timeout */
    uint8_t qsize;               /**< Size of packet buffer queue */
    bool sec_enable;             /**< Encrypt espnow data payload when send and decrypt when receive */
    struct {
        bool ack;
        bool forward;
        bool group;
        bool provisoning;
        bool control_bind;
        bool control_data;
        bool ota_status;
        bool ota_data;
        bool debug_log;
        bool debug_command;
        bool data;
        bool sec_status;
        bool sec;
        bool reserved;
    } receive_enable;            /**< Receive status of packet type */
} espnow_config_t;

#define ESPNOW_INIT_CONFIG_DEFAULT() { \
    .pmk = "ESP_NOW", \
    .forward_enable = true, \
    .send_retry_num = 10, \
    .send_max_timeout = pdMS_TO_TICKS(3000),\
    .qsize = 32, \
    .receive_enable = { \
                .ack           = 1, \
                .forward       = 1, \
                .group         = 1, \
                .provisoning   = 0, \
                .control_bind  = 0, \
                .control_data  = 0, \
                .ota_status    = 0, \
                .ota_data      = 0, \
                .debug_log     = 0, \
                .debug_command = 0, \
                .data          = 0, \
                .sec_status    = 0, \
                .sec           = 0, \
                .reserved      = 0, \
                }, \
    }

/**
 * @brief Divide espnow data into multiple pipes
 */
typedef enum {
    ESPNOW_TYPE_ACK,            /**< For reliable data transmission */
    ESPNOW_TYPE_FORWARD,        /**< Set to forward packets */
    ESPNOW_TYPE_GROUP,          /**< Send a packet that sets the group type */
    ESPNOW_TYPE_PROV,           /**< Network configuration packet */
    ESPNOW_TYPE_CONTROL_BIND,   /**< Binding or unbinding packet */
    ESPNOW_TYPE_CONTROL_DATA,   /**< Control data packet */
    ESPNOW_TYPE_OTA_STATUS,     /**< Status packet for rapid upgrade of batch Device */
    ESPNOW_TYPE_OTA_DATA,       /**< Data packet for rapid upgrade of batch Device */
    ESPNOW_TYPE_DEBUG_LOG,      /**< Equipment debugging log packet */
    ESPNOW_TYPE_DEBUG_COMMAND,  /**< Equipment debugging command packet */
    ESPNOW_TYPE_DATA,           /**< User-defined use */
    ESPNOW_TYPE_SECURITY_STATUS,/**< Security status packet */
    ESPNOW_TYPE_SECURITY,       /**< Security handshake packet */
    ESPNOW_TYPE_SECURITY_DATA,  /**< Security packet */
    ESPNOW_TYPE_RESERVED,       /**< Reserved for other function */
    ESPNOW_TYPE_MAX,
} espnow_type_t;

/**
 * @brief Frame header of espnow
 */
typedef struct {
    uint16_t magic;                    /**< Unique identifier of each packet. Packets with the same identifier will be filtered. 0: a random number */
    uint8_t channel              : 4;  /**< Set the channel where the packet is sent, ESPNOW_CHANNEL_CURRENT or ESPNOW_CHANNEL_ALL */
    bool filter_adjacent_channel : 1;  /**< Because espnow is sent through HT20, it can receive packets from adjacent channels */
    bool filter_weak_signal      : 1;  /**< When the signal received by the receiving device is lower than forward_rssi, frame_head data will be discarded */
    bool security                : 1;  /**< The payload data is encrypted if security is true */
    uint16_t                     : 4;  /**< Reserved */

    /**
     * @brief Configure broadcast
     */
    bool broadcast              : 1;  /**< Packet sent in broadcast mode or unicast mode */
    bool group                  : 1;  /**< Only the group set as broadcast transmission mode is valid */
    bool ack                    : 1;  /**< Wait for the receiving device to return ack to ensure transmission reliability */
    uint16_t retransmit_count   : 5;  /**< Too many packet retransmissions will lead to network congestion */
    uint8_t forward_ttl         : 5;  /**< Number of hops in data transfer */
    int8_t forward_rssi         : 8;  /**< When the data packet signal received by the receiving device is lower than forward_rssi, it will not be transferred,
                                           in order to avoid network congestion caused by packet transfer */
} __attribute__((packed)) espnow_frame_head_t;

#define ESPNOW_FRAME_CONFIG_DEFAULT() \
    { \
        .broadcast = true, \
        .retransmit_count = 10, \
    }

/**
 * @brief When used for unicast, add the target device
 *
 * @param[in]  addr  peer MAC address
 * @param[in]  lmk  peer local master key that is used to encrypt data.
 *              It can be null or ESP_NOW_KEY_LEN length data
 */
esp_err_t espnow_add_peer(const espnow_addr_t addr, const uint8_t *lmk);

/**
 * @brief When used for unicast, delete the target device
 *
 * @param[in]     peer MAC address
 * 
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_del_peer(const espnow_addr_t addr);

/**
 * @brief   Send ESPNOW data
 *
 * @param[in]   type  espnow data type defined by espnow_type_t
 * @param[in]   dest_addr  destination mac address
 * @param[in]   data  the sending data which must not be NULL
 * @param[in]   size  the maximum length of data, must be no more than ESPNOW_DATA_LEN
 * @param[in]   frame_config  if frame_config is NULL, Use ESPNOW_FRAME_CONFIG_DEFAULT configuration
 * @param[in]   wait_ticks  the maximum sending time in ticks
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 *    - ESP_ERR_TIMEOUT
 *    - ESP_ERR_WIFI_TIMEOUT
 */
esp_err_t espnow_send(espnow_type_t type, const espnow_addr_t dest_addr, const void *data,
                      size_t size, const espnow_frame_head_t *frame_config, TickType_t wait_ticks);

/**
 * @brief   ESPNOW type data receive callback function
 *
 * @param[in]  src_addr  peer MAC address
 * @param[in]  data  received data
 * @param[in]  size  length of received data
 * @param[in]  rx_ctrl  received packet radio metadata header
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
typedef esp_err_t (*type_handle_t)(uint8_t *src_addr, void *data,
                      size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl);

/**
 * @brief De-initialize ESPNOW function
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_ESPNOW_NOT_INIT
 */
esp_err_t espnow_deinit(void);

/**
 * @brief Initialize ESPNOW function
 *
 * @param[in]  config  configuration of espnow
 * 
 * @return
 *    - ESP_OK
 *    - ESP_FAIL
 */
esp_err_t espnow_init(const espnow_config_t *config);

/**
 * @brief Set the type data receive status and callback function
 *
 * @param[in]  type  data type defined by espnow_type_t
 * @param[in]  enable  enable or disable the data receive, false - disable, true - enable
 * @param[in]  handle  the receive callback function
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_set_type(espnow_type_t type, bool enable, type_handle_t handle);

/**
 * @brief Get the type data receive status
 *
 * @param[in]  type  data type defined by espnow_type_t
 * @param[out]  enable  store the current receive status of the type data
 * 
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_get_type(espnow_type_t type, bool *enable);

/**
 * @brief      Set group ID addresses
 *
 * @param[in]  group_id  pointer to new group ID addresses
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_add_group(const espnow_group_t group_id);

/**
 * @brief      Delete group ID addresses
 *
 * @param[in]  group_id  pointer to deleted group ID address
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_del_group(const espnow_group_t group_id);

/**
 * @brief      Get the number of group ID addresses
 *
 * @return     the number of group ID addresses
 */
int espnow_get_group_num(void);

/**
 * @brief      Get group ID addresses
 *
 * @param[out] group_id_list  pointer to group ID addresses
 * @param[in]  num  the number of group ID addresses
 *
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_get_group_list(espnow_group_t *group_id_list, size_t num);

/**
 * @brief      Check whether the specified group address is my group
 *
 * @param[in]  group_id  pointer to the specified group ID addresses
 * 
 * @return     true/false
 */
bool espnow_is_my_group(const espnow_group_t group_id);

/**
 * @brief       Dynamically set the grouping of devices through commands
 *
 * @param[in]   addrs_list  mac address list of the grouping devices
 * @param[in]   addrs_num  number of the grouping devices
 * @param[in]   group_id  pointer to the specified group ID addresses
 * @param[in]   frame_head  use ESPNOW_FRAME_CONFIG_DEFAULT configuration if frame_config is NULL
 * @param[in]   type  true: add group, false: delete group
 * @param[in]   wait_ticks  the maximum sending time in ticks
 * 
 * @return
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_send_group(const espnow_addr_t *addrs_list, size_t addrs_num,
                            const espnow_group_t group_id, espnow_frame_head_t *frame_head,
                            bool type, TickType_t wait_ticks);

/**
 * @brief Set the security key info
 *        The security key info is used to derive key and stored to flash.
 *        The derived key is used to encrypt espnow data payload when send and decrypt espnow data payload when receive.
 *
 * @attention Set sec_enable in espnow_config to true when espnow initializes, or the function will return failed.
 * 
 * @param[in]  key_info  security key info
 * 
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_set_key(uint8_t key_info[APP_KEY_LEN]);

/**
 * @brief Get the security key info stored in flash
 *        If no security key info is stored in flash, the function will return failed.
 *
 * @param[out]  key_info  security key info
 * 
 *    - ESP_OK
 *    - ESP_ERR_INVALID_ARG
 */
esp_err_t espnow_get_key(uint8_t key_info[APP_KEY_LEN]);

/**
 * @brief Erase the security key info stored in flash
 *
 *    - ESP_OK
 *    - ESP_ERR_NVS_NOT_FOUND
 */
esp_err_t espnow_erase_key(void);
#ifdef __cplusplus
}
#endif /**< _cplusplus */
