#pragma once

#ifndef PC_SIM
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_err.h"
#endif

#include <time.h>

void sntp_srv_set_timezone(const char* tz);

esp_err_t sntp_srv_sync();
extern time_t sntp_last_sync;
extern struct tm sntp_datetime;


typedef void (*sntp_srv_minutely_update_cb_t)(struct tm* );
extern sntp_srv_minutely_update_cb_t sntp_srv_minutely_update_cb;
esp_err_t sntp_srv_init(const char* tz,  sntp_srv_minutely_update_cb_t cb);