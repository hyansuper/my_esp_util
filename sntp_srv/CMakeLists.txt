idf_component_register(SRCS "sntp_srv.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_common esp_netif)


if(PROJ_REGION STREQUAL "CN")
    add_compile_definitions(SNTP_SRV_SERVER_DEFAULT="ntp.aliyun.com" SNTP_SRV_SERVER_FALLBACK="time.windows.com")
else()
    add_compile_definitions(SNTP_SRV_SERVER_DEFAULT="time.apple.com" SNTP_SRV_SERVER_FALLBACK="time.windows.com")
endif()


# ntp.ntsc.ac.cn
# time.apple.com
# time.asia.apple.com
# cn.pool.ntp.org
# time.windows.com
# ntp.aliyun.com