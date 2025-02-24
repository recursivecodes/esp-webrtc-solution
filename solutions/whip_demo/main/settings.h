/* General settings

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Board name setting refer to `codec_board` README.md for more details
 */
#define TEST_BOARD_NAME "ESP32_P4_DEV_V14"

/**
 * @brief  Video resolution settings
 */
#define VIDEO_WIDTH  1024
#define VIDEO_HEIGHT 600
#define VIDEO_FPS    10

/**
 * @brief  Set for wifi ssid
 */
#define WIFI_SSID "XXXX"

/**
 * @brief  Set for wifi password
 */
#define WIFI_PASSWORD "XXXX"

/**
 * @brief  WHIP server URL
 */
#define WHIP_SERVER "http://XXXX/whip"

/**
 * @brief  WHIP server access token
 */
#define WHIP_TOKEN "username:password"

#ifdef __cplusplus
}
#endif
