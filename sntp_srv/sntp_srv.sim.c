#include "xdial.h"

void sntp_srv_set_timezone(const char* tz) {}

esp_err_t sntp_srv_sync() {
    return ESP_OK;
}

time_t sntp_last_sync;
struct tm sntp_datetime;


void lv_timer_cb(lv_timer_t* timer) {
    time_t now = time(NULL);
    localtime_s(&sntp_datetime, &now);
//    if(sntp_srv_updated_cb)
        sntp_srv_minutely_update_cb(&sntp_datetime);
}

esp_err_t sntp_srv_init(const char* tz) {

    #ifndef FREEZE
    sntp_last_sync = time(NULL);
    lv_timer_cb(NULL);

    lv_timer_create(lv_timer_cb, 60000,  NULL);
    #else
    get_datetime(&sntp_datetime);
    #endif
    return ESP_OK;
}
