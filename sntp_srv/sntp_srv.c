#include "sntp_srv.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

//static const char* const TAG = "sntp";

/*
menuconfig -> component -> LWIP -> SNTP -> max number of servers -> 2

ntp.ntsc.ac.cn
time.apple.com
time.asia.apple.com
cn.pool.ntp.org
time.windows.com
ntp.aliyun.com"
*/


// #define SNTP_SRV_ADJTIME_INTERVAL 2000
// #define SNTP_SRV_ADJTIME_TASK_PRIO 0
// #define SNTP_SRV_ADJTIME_TASK_STACK 1024
const static char* const TAG = "sntp_srv";

struct tm sntp_datetime;
time_t sntp_last_sync;
sntp_srv_minutely_update_cb_t sntp_srv_minutely_update_cb;

static StaticTimer_t timer_buffer;
static TimerHandle_t timer;

static void timer_cb(TimerHandle_t timer) {
    if(uxTimerGetReloadMode(timer) == pdFALSE) {
        vTimerSetReloadMode(timer, pdTRUE);
        xTimerChangePeriod(timer, pdMS_TO_TICKS(60000), portMAX_DELAY);
    }
    time_t now = time(NULL);
    localtime_r(&now, &sntp_datetime);
    if(sntp_srv_minutely_update_cb)
        sntp_srv_minutely_update_cb(&sntp_datetime);
}

static void on_sync(struct timeval* tv) {
    sntp_last_sync = tv->tv_sec;
    localtime_r(&sntp_last_sync, &sntp_datetime);
    unsigned long remain_ticks = pdMS_TO_TICKS((60-sntp_datetime.tm_sec)*1000);
    if(timer) {
        vTimerSetReloadMode(timer, pdFALSE);
        xTimerChangePeriod(timer, remain_ticks, portMAX_DELAY);
    } else {
        timer = xTimerCreateStatic( TAG,
                                remain_ticks, // interval
                                pdFALSE, // auto reload
                                NULL, // timer ID
                                timer_cb,
                                &timer_buffer);

        if(pdPASS != xTimerStart(timer, portMAX_DELAY))
            ESP_LOGE(TAG, "start timer");
    }


    if(sntp_srv_minutely_update_cb)
        sntp_srv_minutely_update_cb(&sntp_datetime);
}

esp_err_t sntp_srv_init(const char* tz, sntp_srv_minutely_update_cb_t cb) {
    if(tz) {
        setenv("TZ", tz, 1);
        tzset();
    }

#if CONFIG_LWIP_SNTP_MAX_SERVERS == 1
    esp_sntp_config_t sntp_conf = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
#elif CONFIG_LWIP_SNTP_MAX_SERVERS >= 2
    esp_sntp_config_t sntp_conf = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2, ESP_SNTP_SERVER_LIST(CONFIG_SNTP_SRV_SERVER_DEFAULT,
                                                                                            #ifdef APP_REGION_CN
                                                                                                "ntp.aliyun.com"
                                                                                            #else
                                                                                                CONFIG_SNTP_SRV_SERVER_FALLBACK
                                                                                            #endif
                                                                                                ));
#endif
    sntp_conf.sync_cb = on_sync;
    sntp_srv_minutely_update_cb = cb;
    return esp_netif_sntp_init(&sntp_conf) || sntp_srv_sync();
}

esp_err_t sntp_srv_sync() {
    esp_err_t err = ESP_OK;
    uint8_t retry = CONFIG_SNTP_SRV_MAX_RETRY;
    while (ESP_ERR_TIMEOUT == (err= esp_netif_sntp_sync_wait(CONFIG_SNTP_SRV_SYNC_TIMEOUT / portTICK_PERIOD_MS))
            && retry --);
    return err;
}

void sntp_srv_set_timezone(const char* tz) {
    setenv("TZ", tz, 1);
    tzset();
    time_t now = time(NULL);
    localtime_r(&now, &sntp_datetime);

    if(sntp_srv_minutely_update_cb)
        sntp_srv_minutely_update_cb(&sntp_datetime);
}
