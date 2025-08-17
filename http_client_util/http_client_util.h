#pragma once

#include "esp_err.h"
#include "esp_http_client.h"

esp_http_client_handle_t http_client_util_create();
static inline void http_client_util_delete(esp_http_client_handle_t client) {
    esp_http_client_cleanup(client);
}

// blocking
esp_err_t http_client_util_get(esp_http_client_handle_t client, char* buf, int* len, const char* fmt, ...);
esp_err_t http_client_util_get_gzip(esp_http_client_handle_t client, char* buf, int* len, char** gbuf, int* glen, const char* fmt, ...);

esp_err_t http_client_util_get_v(esp_http_client_handle_t client, char* buf, int* len, const char* url_fmt, va_list args);
esp_err_t http_client_util_get_gzip_v(esp_http_client_handle_t client, char* buf, int* len, char** gbuf, int* glen, const char* url_fmt, va_list args);

esp_err_t http_client_util_post_v(esp_http_client_handle_t client, char* buf, int* len, const char* post_body, size_t post_len, const char* fmt, va_list args);
esp_err_t http_client_util_post(esp_http_client_handle_t client, char* buf, int* len, const char* post_body, size_t post_len, const char* fmt, ...);