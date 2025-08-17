idf_component_register(SRCS "dns_server.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_common esp_netif)