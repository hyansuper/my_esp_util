#pragma once

#ifndef PC_SIM
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_err.h"
#else
#define wifi_config_t int
#endif

#include <stdbool.h>

#define WMGR_BIT_WIFI_CONNECTED 1
#define WMGR_BIT_WIFI_STA_FAIL      (1<<1)


typedef struct {
	bool connected;
	const char* ipstr;
	const char* hostname;
	const char* macstr;
} wifi_mgr_state_t;

typedef void (*wifi_mgr_cb_t)(wifi_mgr_state_t* state);
extern wifi_mgr_state_t wifi_mgr_state;
extern wifi_mgr_cb_t wifi_mgr_sta_conn_cb, wifi_mgr_prov_cb;

esp_err_t wifi_mgr_sta(wifi_mgr_cb_t sta_cb);
esp_err_t wifi_mgr_prov(wifi_mgr_cb_t prov_cb);
esp_err_t wifi_mgr_prov_connect(wifi_config_t* cfg);
void wifi_mgr_init();


static inline bool wifi_mgr_connected() {return wifi_mgr_state.connected;}
static inline const char* wifi_mgr_hostname() {return wifi_mgr_state.hostname;}
static inline const char* wifi_mgr_ipstr() {return wifi_mgr_state.ipstr;}
static inline const char* wifi_mgr_macstr() {return wifi_mgr_state.macstr;}
int wifi_mgr_get_sta_rssi();