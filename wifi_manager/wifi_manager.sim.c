#include "wifi_manager.h"

#define WMGR_BIT_WIFI_CONNECTED 1
#define WMGR_BIT_WIFI_STA_FAIL      (1<<1)

esp_err_t wmgr_sta() {
    return ESP_OK;
}
esp_err_t wmgr_prov() {
    return ESP_OK;
}
esp_err_t wmgr_prov_connect(wifi_config_t* cfg) {
    return ESP_OK;
}
void wmgr_init() {
    return ESP_OK;
}

int wmgr_get_sta_rssi() {
    return 0;
}
wmgr_state_t wmgr_state = {
    .connected = true,
    .ipstr = "192.168.1.10",
    .hostname = "XDial-abcd",
};
wmgr_cb_t wmgr_sta_conn_cb, wmgr_prov_cb;
