idf_component_register(SRCS "server_util.c" "proxy.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_common esp_http_server http_client_util
                    EMBED_FILES util-min.js.gz favicon.jpg)