#include "http_client_util.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_event.h"
#ifndef CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY
    #include "esp_crt_bundle.h"
#endif

#include "zip.h"

static const char* const TAG = "http_client_util";

typedef struct {
    void* buf;
    size_t size; // total size of buf
    size_t len; // actual size used
    esp_err_t err;
} http_client_util_buffer_t;


/**
 * event: connected - (header_sent - header*n - data*n - finish)*n - disconected
 * the process goes on reguardless of return code.
 * unless manually call esp_http_client_close(evt->client)
 */
static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    http_client_util_buffer_t* buffer = evt->user_data;
    switch(evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
        buffer->err = ESP_FAIL;
        break;
    case HTTP_EVENT_ON_DATA:
        if(buffer->buf) {
            if(evt->data_len< (buffer->size - buffer->len)) {
                memcpy(buffer->buf + buffer->len, evt->data, evt->data_len);
                buffer->len += evt->data_len;
            } else {
                ESP_LOGE(TAG, "inbuf too small");
                buffer->err = ESP_ERR_INVALID_SIZE;
            }
        }
        break;

    default: ;
    }
    return ESP_OK;
}

esp_http_client_handle_t http_client_util_create() {

    esp_http_client_config_t cfg = {
        .url="https://www.google.com", // this is only to pass the init, url will change later
        // .keep_alive_enable = true,
        .event_handler = _http_event_handler,
        #ifndef CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY
        .crt_bundle_attach = esp_crt_bundle_attach,
        #endif
        #ifdef CONFIG_HTTP_CLIENT_UTIL_SKIP_COMMON_NAME_CHECK
        .skip_cert_common_name_check = true,
        #endif
    };

    esp_http_client_handle_t client= esp_http_client_init(&cfg);
    return client;
}

esp_err_t http_client_util_perform_v(esp_http_client_handle_t client, char* buf, int *len, const char* url_fmt, va_list args) {
    char url[256];
    if(sizeof(url) <= vsnprintf(url, sizeof(url), url_fmt, args)) {
        ESP_LOGE(TAG, "url too long");
        return ESP_ERR_NO_MEM;
    }
    esp_http_client_set_url(client, url);
    http_client_util_buffer_t inbuf = {.size=*len, .buf=buf};
    esp_http_client_set_user_data(client, &inbuf);
    esp_err_t ret;
    if((ret=esp_http_client_perform(client))) {
        ESP_LOGE(TAG, "perform, err: %d", ret);
    }
    *len = inbuf.len;
    esp_http_client_close(client); // call close on every opened connection!!!!
    if(inbuf.err) ret=inbuf.err;
    return ret;
}

esp_err_t http_client_util_get_v(esp_http_client_handle_t client, char* buf, int* len, const char* url_fmt, va_list args) {
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    return http_client_util_perform_v(client, buf, len, url_fmt, args);
}

esp_err_t http_client_util_get(esp_http_client_handle_t client, char* buf, int* len, const char* fmt, ...) {
    // http_client_util_buffer_t* ret_buf;
    // esp_http_client_get_user_data(client, &ret_buf);
    va_list args;
    va_start(args, fmt);
    esp_err_t ret = http_client_util_get_v(client, buf, len, fmt, args);
    va_end(args);
    return ret;
}

esp_err_t http_client_util_get_gzip_v(esp_http_client_handle_t client, char* buf, int* len, char** gbuf, int* glen, const char* url_fmt, va_list args) {

    esp_http_client_set_header(client, "Accept-Encoding", "gzip");
    esp_err_t ret = http_client_util_get_v(client, buf, len, url_fmt, args);
    esp_http_client_delete_header(client, "Accept-Encoding");
    if(ret) return ret;


    unsigned int crc32, outlen=0, ziplen;
    unsigned char* outbuf, *zip;

    ESP_RETURN_ON_FALSE(ZIP_OK==gunzip_check((const unsigned char*)buf, *len, &zip, &ziplen, &outlen, &crc32), ESP_ERR_INVALID_RESPONSE, TAG, "malformed");
    if(*gbuf) {
        ESP_RETURN_ON_FALSE(outlen<= *glen, ESP_ERR_NO_MEM, TAG, "buf too small to gunzip, %d needed", outlen);
        outbuf = (unsigned char*)(*gbuf);
    } else { // *gbuf can be null, in such case, a malloc-ed mem is returned
        ESP_RETURN_ON_FALSE((outbuf=malloc(outlen)), ESP_ERR_NO_MEM, TAG, "malloc for decompress(%d)", outlen);
    }

    ESP_GOTO_ON_FALSE(0==puff(outbuf, &outlen, zip, &ziplen), ESP_ERR_INVALID_RESPONSE, gzip_finish, TAG, "puff");
    #ifdef CONFIG_ZIP_GUNZIP_CHECK_BODY_CRC32
    ESP_GOTO_ON_FALSE(crc32==calc_crc32(outbuf, outlen), ESP_ERR_INVALID_CRC, gzip_finish, TAG, "crc32");
    #endif

    if(*gbuf==NULL)
        *gbuf = (char*)outbuf;
    *glen = outlen;
    return ret;

gzip_finish:
    if(*gbuf==NULL) // if outbuf is malloc-ed, it should be free-ed on err
        free(outbuf);
    return ret;
}

esp_err_t http_client_util_get_gzip(esp_http_client_handle_t client, char* buf, int* len, char** gbuf, int* glen, const char* fmt, ...) {
    // http_client_util_buffer_t* ret_buf;
    // esp_http_client_get_user_data(client, &ret_buf);
    va_list args;
    va_start(args, fmt);
    esp_err_t ret = http_client_util_get_gzip_v(client, buf, len, gbuf, glen, fmt, args);
    va_end(args);
    return ret;
}

esp_err_t http_client_util_post_v(esp_http_client_handle_t client, char* buf, int* len, const char* post_body, size_t post_len, const char* fmt, va_list args) {
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_body, post_len);
    return http_client_util_perform_v(client, buf, len, fmt, args);
}

esp_err_t http_client_util_post(esp_http_client_handle_t client, char* buf, int* len, const char* post_body, size_t post_len, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    esp_err_t ret = http_client_util_post_v(client, buf, len, post_body, post_len, fmt, args);
    va_end(args);
    return ret;
}