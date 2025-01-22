/* OpenAI signaling

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "https_client.h"
#include "common.h"
#include "esp_log.h"

#define TAG "OPENAI_SIGNALING"

#define OPENAI_REALTIME_URL "https://api.openai.com/v1/realtime?model=gpt-4o-mini-realtime-preview-2024-12-17"

#define SAFE_FREE(p) if (p) {   \
    free(p);                    \
    p = NULL;                   \
}

typedef struct {
    esp_peer_signaling_cfg_t cfg;
    uint8_t                 *remote_sdp;
    int                      remote_sdp_size;
} openai_signaling_t;

static int openai_signaling_start(esp_peer_signaling_cfg_t *cfg, esp_peer_signaling_handle_t *h)
{
    openai_signaling_t *sig = (openai_signaling_t *)calloc(1, sizeof(openai_signaling_t));
    if (sig == NULL) {
        return ESP_PEER_ERR_NO_MEM;
    }
    sig->cfg = *cfg;
    *h = sig;
    esp_peer_signaling_ice_info_t ice_info = {
        .is_initiator = true,
    };
    sig->cfg.on_ice_info(&ice_info, sig->cfg.ctx);
    sig->cfg.on_connected(sig->cfg.ctx);
    return ESP_PEER_ERR_NONE;
}

static void openai_sdp_answer(http_resp_t *resp, void *ctx)
{
    openai_signaling_t *sig = (openai_signaling_t *)ctx;
    printf("Get remote SDP %s\n", (char *)resp->data);
    SAFE_FREE(sig->remote_sdp);
    sig->remote_sdp = (uint8_t *)malloc(resp->size);
    if (sig->remote_sdp == NULL) {
        ESP_LOGE(TAG, "No enough memory for remote sdp");
        return;
    }
    memcpy(sig->remote_sdp, resp->data, resp->size);
    sig->remote_sdp_size = resp->size;
}

static int openai_signaling_send_msg(esp_peer_signaling_handle_t h, esp_peer_signaling_msg_t *msg)
{
    openai_signaling_t *sig = (openai_signaling_t *)h;
    if (msg->type == ESP_PEER_SIGNALING_MSG_BYE) {

    } else if (msg->type == ESP_PEER_SIGNALING_MSG_SDP) {
        printf("Receive local SDP\n");
        char content_type[32] = "Content-Type: application/sdp";
        char auth[128];
        snprintf(auth, 128, "Authorization: Bearer %s", (char *)sig->cfg.extra_cfg);
        char *header[] = {
            content_type,
            auth,
            NULL,
        };
        int ret = https_post(OPENAI_REALTIME_URL, header, (char *)msg->data, openai_sdp_answer, h);
        if (ret != 0 || sig->remote_sdp == NULL) {
            ESP_LOGE(TAG, "Fail to post data to %s", OPENAI_REALTIME_URL);
            return -1;
        }
        esp_peer_signaling_msg_t sdp_msg = {
            .type = ESP_PEER_SIGNALING_MSG_SDP,
            .data = sig->remote_sdp,
            .size = sig->remote_sdp_size,
        };
        sig->cfg.on_msg(&sdp_msg, sig->cfg.ctx);
    }
    return 0;
}

static int openai_signaling_stop(esp_peer_signaling_handle_t h)
{
    openai_signaling_t *sig = (openai_signaling_t *)h;
    sig->cfg.on_close(sig->cfg.ctx);
    SAFE_FREE(sig->remote_sdp);
    SAFE_FREE(sig);
    return 0;
}

const esp_peer_signaling_impl_t *esp_signaling_get_openai_signaling(void)
{
    static const esp_peer_signaling_impl_t impl = {
        .start = openai_signaling_start,
        .send_msg = openai_signaling_send_msg,
        .stop = openai_signaling_stop,
    };
    return &impl;
}