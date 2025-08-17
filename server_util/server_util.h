#pragma once

#include "esp_err.h"
#include <esp_http_server.h>

typedef struct {
	char* path;
	char* start;
	char* end;
	char* encoding;
	char* type;
} server_rsc_t;

#define TYPE_JS "application/javascript"
#define TYPE_ICO "image/x-icon"
#define TYPE_PNG "image/png"
#define TYPE_JPG "image/jpeg"
#define TYPE_HTML "text/html"

extern const server_rsc_t rsc_favicon;
extern const server_rsc_t rsc_util;

void serve_rsc(httpd_handle_t server, server_rsc_t* rsc);

#ifdef CONFIG_HTTP_SERVER_UTIL_ENABLE_PROXY
esp_err_t server_util_proxy_handler(httpd_req_t* req);
#endif