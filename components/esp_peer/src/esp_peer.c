/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

#include "esp_peer.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    esp_peer_ops_t    ops;
    esp_peer_handle_t handle;
    esp_peer_cfg_t    cfg;  // Store config to access callbacks
} peer_wrapper_t;

int esp_peer_open(esp_peer_cfg_t *cfg, const esp_peer_ops_t *ops, esp_peer_handle_t *handle)
{
    if (cfg == NULL || ops == NULL || handle == NULL || ops->open == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = calloc(1, sizeof(peer_wrapper_t));
    if (peer == NULL) {
        return ESP_PEER_ERR_NO_MEM;
    }
    memcpy(&peer->ops, ops, sizeof(esp_peer_ops_t));
    memcpy(&peer->cfg, cfg, sizeof(esp_peer_cfg_t));  // Store config for callback access
    int ret = ops->open(cfg, &peer->handle);
    if (ret != ESP_PEER_ERR_NONE) {
        free(peer);
        return ret;
    }
    *handle = peer;
    return ret;
}

int esp_peer_new_connection(esp_peer_handle_t handle)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.new_connection) {
        return peer->ops.new_connection(peer->handle);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_create_data_channel(esp_peer_handle_t handle, esp_peer_data_channel_cfg_t *ch_cfg)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.create_data_channel) {
        return peer->ops.create_data_channel(peer->handle, ch_cfg);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_close_data_channel(esp_peer_handle_t handle, const char *label)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.close_data_channel) {
        return peer->ops.close_data_channel(peer->handle, label);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_update_ice_info(esp_peer_handle_t handle, esp_peer_role_t role, esp_peer_ice_server_cfg_t* server, int server_num)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.update_ice_info) {
        return peer->ops.update_ice_info(peer->handle, role, server, server_num);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_send_msg(esp_peer_handle_t handle, esp_peer_msg_t *msg)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.send_msg) {
        return peer->ops.send_msg(peer->handle, msg);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_send_video(esp_peer_handle_t handle, esp_peer_video_frame_t *info)
{
    if (handle == NULL || info == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;

    // Call the video send callback if provided (for SEI injection, etc.)
    if (peer->cfg.on_video_send) {
        uint8_t *modified_data = peer->cfg.on_video_send(info, peer->cfg.ctx);
        if (modified_data == NULL) {
            // Callback returned NULL - drop the frame
            return ESP_PEER_ERR_NONE;
        } else if (modified_data != info->data) {
            // Callback returned modified data - update frame
            // Note: Caller is responsible for memory management of modified_data
            info->data = modified_data;
            // Size might have changed - callback should update info->size if needed
        }
        // If modified_data == info->data, use original frame unchanged
    }

    if (peer->ops.send_video) {
        return peer->ops.send_video(peer->handle, info);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_send_audio(esp_peer_handle_t handle, esp_peer_audio_frame_t *info)
{
    if (handle == NULL || info == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.send_audio) {
        return peer->ops.send_audio(peer->handle, info);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_send_data(esp_peer_handle_t handle, esp_peer_data_frame_t *info)
{
    if (handle == NULL || info == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.send_data) {
        return peer->ops.send_data(peer->handle, info);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_main_loop(esp_peer_handle_t handle)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.main_loop) {
        return peer->ops.main_loop(peer->handle);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_disconnect(esp_peer_handle_t handle)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.disconnect) {
        return peer->ops.disconnect(peer->handle);
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_query(esp_peer_handle_t handle)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    if (peer->ops.query) {
        peer->ops.query(peer->handle);
        return ESP_PEER_ERR_NONE;
    }
    return ESP_PEER_ERR_NOT_SUPPORT;
}

int esp_peer_close(esp_peer_handle_t handle)
{
    if (handle == NULL) {
        return ESP_PEER_ERR_INVALID_ARG;
    }
    peer_wrapper_t *peer = (peer_wrapper_t *)handle;
    int ret = ESP_PEER_ERR_NOT_SUPPORT;
    if (peer->ops.close) {
        ret = peer->ops.close(peer->handle);
    }
    free(peer);
    return ret;
}
