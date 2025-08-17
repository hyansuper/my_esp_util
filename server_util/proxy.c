#include "server_util.h"

#ifdef CONFIG_HTTP_SERVER_UTIL_ENABLE_PROXY
#include "http_client_util.h"
#include "esp_log.h"

#define PROXY_TSK_STACK (1024*4)
#define PROXY_TSK_PRIO (1)

static const char* const TAG = "proxy";

typedef struct {
	httpd_req_t* req;
	char* url;
} server_util_proxy_config_t;

static void proxy_tsk_cb(server_util_proxy_config_t* proxy) {
	ESP_LOGI(TAG, "proxy-ing: %s", proxy->url);
	char* err_msg = NULL;
	esp_http_client_handle_t client = http_client_util_create();
	if(client==NULL) {
		ESP_LOGE(TAG, "create client");
		err_msg = "err: no mem";
		goto err;
	}

	esp_err_t ret = http_client_util_perform(client, proxy->url);
	if(ret) {
		ESP_LOGE(TAG, "client perform err: %d", ret);
		err_msg = "err: fetching url";
		goto err;
	}

	http_client_util_buffer_t* inbuf;
    esp_http_client_get_user_data(client, &inbuf);
	httpd_resp_send(proxy->req, inbuf->buf, inbuf->len);

cleanup:
	http_client_util_delete(client);
	httpd_req_async_handler_complete(proxy->req);
	// free(proxy->url); // url is malloced together with proxy struct
	free(proxy);
	vTaskDelete(NULL);
	return;

err:
	httpd_resp_send_err(proxy->req, HTTPD_500_INTERNAL_SERVER_ERROR, err_msg);
	goto cleanup;
}

esp_err_t server_util_proxy_handler(httpd_req_t* req) {
	char* err_msg = NULL;

	size_t len = httpd_req_get_url_query_len(req)+1;
	server_util_proxy_config_t* proxy = malloc(sizeof(server_util_proxy_config_t)+len);

	if(proxy==NULL) {
		err_msg = "malloc proxy config";
		goto handler_err;
	}
	
	proxy->url = proxy + sizeof(server_util_proxy_config_t);
	proxy->url[0] = 0;
	proxy->req = NULL;

	if(httpd_req_get_url_query_str(req, proxy->url, len)) {
		err_msg = "wrong query";
		goto handler_err;
	}

	if(httpd_req_async_handler_begin(req, &proxy->req)
			|| pdPASS!= xTaskCreate( proxy_tsk_cb,
                         "proxy",
                         PROXY_TSK_STACK,
                         proxy,
                         PROXY_TSK_PRIO,
                         NULL )) {

		err_msg = "create proxy task";
		goto handler_err;
	}
	return ESP_OK;

handler_err:
	if(proxy) {
		httpd_resp_send_err(proxy->req, HTTPD_500_INTERNAL_SERVER_ERROR, err_msg);
		httpd_req_async_handler_complete(proxy->req); // THIS FUNC CHECK FOR NULL
		free(proxy);
	}
	return ESP_FAIL;
}

#endif