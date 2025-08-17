#include "server_util.h"


extern const char util_start[] asm("_binary_util_min_js_gz_start");
extern const char util_end[] asm("_binary_util_min_js_gz_end");

// extern const char util_start[] asm("_binary_util_js_start");
// extern const char util_end[] asm("_binary_util_js_end");

extern const char favicon_start[] asm("_binary_favicon_jpg_start");
extern const char favicon_end[] asm("_binary_favicon_jpg_end");


const server_rsc_t rsc_util = {.path="/util-min.js.gz", .start=util_start, .end=util_end, .type=TYPE_JS, .encoding="gzip"};
const server_rsc_t rsc_favicon = {.path="/favicon.jpg", .start=favicon_start, .end=favicon_end, .type=TYPE_JPG};//, .encoding="gzip"};

static esp_err_t serve_rsc_handler(httpd_req_t *req) {
    server_rsc_t* rsc = req->user_ctx;
    if(rsc->type) httpd_resp_set_type(req, rsc->type);
    if(rsc->encoding) httpd_resp_set_hdr(req, "Content-Encoding", rsc->encoding);
    httpd_resp_send(req, rsc->start, rsc->end - rsc->start);
    return ESP_OK;
}

void serve_rsc(httpd_handle_t server, server_rsc_t* rsc) {
	httpd_uri_t httpd_uri = {
		.uri      = rsc->path,
        .method   = HTTP_GET,
        .handler  = serve_rsc_handler,
        .user_ctx = rsc,
	};
	httpd_register_uri_handler(server, &httpd_uri);
}

