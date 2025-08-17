idf_component_register(SRCS "http_client_util.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_common mbedtls mbedtls zip
                       REQUIRES esp_http_client)
