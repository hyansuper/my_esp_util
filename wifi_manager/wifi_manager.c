#include "wifi_manager.h"

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include <string.h>
#include "server_util.h"
#include "dns_server.h"
#include <netdb.h>
#include "ext_mjson.h"

#ifdef APP_REGION_CN
extern const char prov_portal_start[] asm("_binary_prov_portal_cn_html_gz_start");
extern const char prov_portal_end[] asm("_binary_prov_portal_cn_html_gz_end");
#else
extern const char prov_portal_start[] asm("_binary_prov_portal_en_html_gz_start");
extern const char prov_portal_end[] asm("_binary_prov_portal_en_html_gz_end");
#endif

const static server_rsc_t rsc_portal_index = {.path="/", .start=prov_portal_start, .end=prov_portal_end, .type="text/html", .encoding="gzip"};

const static char* const TAG = "wifi_manager";

wifi_mgr_cb_t wifi_mgr_sta_conn_cb, wifi_mgr_prov_cb;

static EventGroupHandle_t wifi_mgr_event_group;

static char ipstr[16] = "\0";
static char macstr[18] = "\0";

#ifdef CONFIG_WMGR_HOSTNAME_CUSTOM
#define HOSTNAME CONFIG_WMGR_HOSTNAME_CUSTOM_NAME
#else
#define HOSTNAME APP_NAME
#endif

#ifdef CONFIG_WMGR_APPEND_DEVICE_ID_TO_HOSTNAME
    static char hostname[] = HOSTNAME "-0000";
#else
    static char hostname[] = HOSTNAME;
#endif

wifi_mgr_state_t wifi_mgr_state = {
    .ipstr = ipstr,
    .hostname = hostname,
    .macstr = macstr,
};
static esp_err_t _nvs_init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase() || nvs_flash_init();
    }
    return err;
}

void wifi_mgr_init(char* c) {
    uint8_t mac[6];
    // esp_base_mac_addr_get(mac);
    esp_efuse_mac_get_default(mac);
    sprintf(macstr, MACSTR, MAC2STR(mac));
    #ifdef CONFIG_WMGR_APPEND_DEVICE_ID_TO_HOSTNAME
        sprintf(hostname+sizeof(hostname)-5, "%02X%02X", mac[4],mac[5]);
    #endif
}

static void wifi_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static unsigned int s_retry_num = 0;
    static TimerHandle_t xTimer = NULL;
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    bool is_sta = mode == WIFI_MODE_STA;
    if (event_base == WIFI_EVENT) {
        if(event_id == WIFI_EVENT_STA_START && is_sta) {
            esp_wifi_connect();

        } else if(event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI(TAG, "wifi retry=%d", s_retry_num);
            if (/* is_sta && */ s_retry_num < CONFIG_WMGR_CONN_MAX_RETRY) {
                esp_wifi_connect();
                s_retry_num ++;
            } else {
                if(wifi_mgr_event_group)
                    xEventGroupSetBits(wifi_mgr_event_group, WMGR_BIT_WIFI_STA_FAIL);
                wifi_mgr_state.connected = false;
                if(is_sta) {
                    if(s_retry_num>= CONFIG_WMGR_CONN_MAX_RETRY && wifi_mgr_sta_conn_cb) {
                        wifi_mgr_sta_conn_cb(&wifi_mgr_state);
                    }
                    if(xTimer==NULL && pdPASS!= xTimerStart( (xTimer= xTimerCreate( TAG,
                                                pdMS_TO_TICKS(CONFIG_WMGR_STA_RECONN_INTERVAL), // interval
                                                pdTRUE, // auto reload
                                                NULL, // timer ID
                                                esp_wifi_connect)),
                                    pdMS_TO_TICKS(5000))) {
                        ESP_LOGE(TAG, "create timer");
                        esp_restart();
                    }
                }
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        sprintf(ipstr, IPSTR, IP2STR(&((ip_event_got_ip_t*)event_data)->ip_info.ip));
        s_retry_num = 0;
        if(xTimer) {
            xTimerDelete(xTimer, portMAX_DELAY);
            xTimer = NULL;
        }
        if(wifi_mgr_event_group)
            xEventGroupSetBits(wifi_mgr_event_group, WMGR_BIT_WIFI_CONNECTED);
        wifi_mgr_state.connected = true;
        if(is_sta && wifi_mgr_sta_conn_cb) {
            wifi_mgr_sta_conn_cb(&wifi_mgr_state);
        }
    }
}

static esp_err_t wifi_mgr_start_common() {
    if(wifi_mgr_event_group == NULL) {

        ESP_RETURN_ON_ERROR(_nvs_init(), TAG, "init nvs");

        wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_RETURN_ON_ERROR(esp_wifi_init(&init_cfg), TAG, "init wifi");

        wifi_mgr_event_group = xEventGroupCreate();

        esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &wifi_handler,
                                            wifi_mgr_event_group,
                                            NULL);

        esp_event_handler_instance_register(IP_EVENT,
                                            IP_EVENT_STA_GOT_IP,
                                            &wifi_handler,
                                            wifi_mgr_event_group,
                                            NULL);
        esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
        esp_netif_set_hostname(sta_netif, hostname);

    }
    return ESP_OK;

}
esp_err_t wifi_mgr_sta(wifi_mgr_cb_t cb) {
    wifi_mgr_sta_conn_cb = cb;
    ESP_RETURN_ON_ERROR(wifi_mgr_start_common(), TAG, "");

    esp_wifi_set_mode(WIFI_MODE_STA);
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "start wifi");
    EventBits_t bits = xEventGroupWaitBits(wifi_mgr_event_group,
                        WMGR_BIT_WIFI_CONNECTED | WMGR_BIT_WIFI_STA_FAIL,
                        pdTRUE,
                        pdFALSE,
                        pdMS_TO_TICKS(CONFIG_WMGR_WAIT_CONN_TIMEOUT));
    if(bits & WMGR_BIT_WIFI_CONNECTED) {
        nvs_flash_deinit();
        vEventGroupDelete(wifi_mgr_event_group);
        wifi_mgr_event_group = NULL;
        return ESP_OK;
    }

    return bits? ESP_FAIL: ESP_ERR_TIMEOUT;
}

static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // Set status
    httpd_resp_set_status(req, "302 Found");
    // Redirect to the "/" root directory
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t http_prov_conn_handler(httpd_req_t *req) {
    char*  buf;
    if(NULL == (buf= malloc(req->content_len + 1))) {
        return ESP_ERR_NO_MEM;
    }
    size_t off = 0;
    // while (off < req->content_len) {
        /* Read data received in the request */
        int ret = httpd_req_recv(req, buf + off, req->content_len - off);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            }
            free (buf);
            return ESP_FAIL;
        }
        off += ret;
    // }
    buf[off] = '\0';
    ESP_LOGI(TAG, "%s", buf);
    wifi_config_t cfg_sta ={  // the brackets to zero initialize
            .sta={
                .pmf_cfg={
                    .capable=true,
                    .required=false}
            }
        };
    if(0<mjson_get_string(buf, off, "$.ssid", (char*)cfg_sta.sta.ssid, sizeof(cfg_sta.sta.ssid))
        && 0<=mjson_get_string(buf, off, "$.pw", (char*)cfg_sta.sta.password, sizeof(cfg_sta.sta.password)))
    {
        // ESP_LOGI(TAG, "ssid: %s, pw: %s", (char*)cfg_sta.sta.ssid, (char*)cfg_sta.sta.password);
        esp_wifi_set_config(WIFI_IF_STA, &cfg_sta);
        wifi_mgr_state.connected= false;
        xEventGroupClearBits(req->user_ctx, WMGR_BIT_WIFI_CONNECTED | WMGR_BIT_WIFI_STA_FAIL);
        ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "prov connect");
    }
    free(buf);

    EventBits_t bits = xEventGroupWaitBits(req->user_ctx,
            WMGR_BIT_WIFI_CONNECTED | WMGR_BIT_WIFI_STA_FAIL,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(CONFIG_WMGR_WAIT_CONN_TIMEOUT));

    char out[]= "{\"success\":0}";
    if(bits & WMGR_BIT_WIFI_CONNECTED)
        out[11] = '1';
    httpd_resp_send(req, out, HTTPD_RESP_USE_STRLEN);
    if(wifi_mgr_prov_cb)
        wifi_mgr_prov_cb(&wifi_mgr_state);
    return ESP_OK;
}

esp_err_t wifi_mgr_prov(wifi_mgr_cb_t prov_cb) {
    wifi_mgr_prov_cb = prov_cb;
    esp_wifi_stop();
    ESP_RETURN_ON_ERROR(wifi_mgr_start_common(), TAG, "");
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    wifi_config_t cfg_ap = {
        .ap = {
            .ssid_len = strlen(hostname),
            #if CONFIG_WMGR_PROV_MAX_CLIENT >0
            .max_connection = CONFIG_WMGR_PROV_MAX_CLIENT,
            #endif
            .authmode = WIFI_AUTH_OPEN,
            .pmf_cfg = {.required = true,},
        },
    };
    strcpy((char*)cfg_ap.ap.ssid, hostname);
    esp_netif_t* ap_netif = esp_netif_create_default_wifi_ap();

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(ap_netif, &ip_info);
    sprintf(ipstr, IPSTR, IP2STR(&ip_info.ip));

    esp_wifi_set_config(WIFI_IF_AP, &cfg_ap);
    esp_netif_set_hostname(ap_netif, hostname);

    // esp_netif_ip_info_t ip_info;
    // ip_info.ip.addr=ipaddr_addr("1.2.3.4");
    // ip_info.gw.addr=ipaddr_addr("1.2.3.4");
    // ip_info.netmask.addr=ipaddr_addr("255.255.255.0");
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(ap_netif)
    //                             ||esp_netif_set_ip_info(ap_netif, &ip_info)
    //                             ||esp_netif_dhcps_start(ap_netif));

    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "start wifi");

    // start http server for provision
    httpd_handle_t server = NULL;
    httpd_config_t httpd_cfg = HTTPD_DEFAULT_CONFIG();
    // httpd_cfg.max_req_hdr_len = 2048;
    // httpd_cfg.max_uri_len = 2048;
    ESP_RETURN_ON_ERROR(httpd_start(&server, &httpd_cfg), TAG, "start server");

    serve_rsc(server, &rsc_util);
    serve_rsc(server, &rsc_portal_index);
    httpd_uri_t uri_prov_conn = {
        .uri      = "/conn",
        .method   = HTTP_POST,
        .handler  = http_prov_conn_handler,
        .user_ctx = wifi_mgr_event_group,
    };
    httpd_register_uri_handler(server, &uri_prov_conn);
    httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);

    // DNS server
    dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
    dns_server_handle_t dns_handle = start_dns_server(&config);

    return dns_handle? ESP_OK: ESP_ERR_NO_MEM;
}

esp_err_t wifi_mgr_prov_connect(wifi_config_t* cfg) {
    esp_wifi_set_config(WIFI_IF_STA, cfg);
    ESP_RETURN_ON_ERROR(esp_wifi_connect(), TAG, "prov connect");
    return ESP_OK;
}

int wifi_mgr_get_sta_rssi() {
    if(wifi_mgr_state.connected) {
        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
        return ap_info.rssi;
    }
    return 0;
}