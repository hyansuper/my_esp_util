#include "kv.h"

esp_err_t kv_init(){return ESP_OK;}
esp_err_t kv_deinit() {return ESP_OK;}
esp_err_t kv_open(const char* ns, nvs_open_mode_t mode, nvs_handle_t *h){return ESP_OK;}
esp_err_t kv_close(nvs_handle_t h){return ESP_OK;}
esp_err_t kv_erase_all(){return ESP_OK;}
esp_err_t kv_save(const char* ns, const char* key, const void* val, size_t len){return ESP_OK;}
esp_err_t kv_save_u8(const char* ns, const char* key, uint8_t val){return ESP_OK;}
