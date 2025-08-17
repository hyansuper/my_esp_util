#pragma once

#ifndef PC_SIM
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#else
typedef int nvs_handle_t;
typedef int nvs_open_mode_t;
#include <stdint.h>
#endif

#ifdef CONFIG_KV_PART_CUSTOM
#define KV_PART CONFIG_KV_PART_CUSTOM_NAME
#else
#define KV_PART APP_NAME"_nvs"
#endif


esp_err_t kv_init();
esp_err_t kv_deinit();
esp_err_t kv_open(const char* ns, nvs_open_mode_t mode, nvs_handle_t *h);
esp_err_t kv_close(nvs_handle_t hl);
esp_err_t kv_erase_all();
esp_err_t kv_save(const char* ns, const char* key, const void* val, size_t len);
esp_err_t kv_save_u8(const char* ns, const char* key, uint8_t val);
esp_err_t kv_get_blob_strict(const nvs_handle_t hl, const char* key, void* blob, size_t sz);

esp_err_t kv_malloc_str(const nvs_handle_t hl, const char* key, char** out);