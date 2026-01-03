if(PROJ_LANG STREQUAL "ZH")
    set(embed_files prov_portal_zh.html.gz)
else()
    set(embed_files prov_portal_en.html.gz)
endif()

idf_component_register(SRCS "wifi_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi
                    PRIV_REQUIRES esp_common nvs_flash esp_http_server server_util mjson dns_server
                    EMBED_FILES ${embed_files})
