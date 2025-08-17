#include "kv.h"
#include "esp_log.h"

static const char* TAG = "kv";

static bool inited = false;

esp_err_t kv_init() {
	esp_err_t err = ESP_OK;
	if(inited) return err;
	err = nvs_flash_init_partition(KV_PART);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        err = nvs_flash_erase_partition(KV_PART) || nvs_flash_init_partition(KV_PART);
    }
    inited = err==ESP_OK;
    return err;
}

esp_err_t kv_deinit() {
	esp_err_t err = ESP_OK;
	if(!inited) return err;
	err = nvs_flash_deinit_partition(KV_PART);
	inited = !(err==ESP_OK);
	return err;
}

esp_err_t kv_open(const char* ns, nvs_open_mode_t mode, nvs_handle_t *h)
{
	esp_err_t err = ESP_OK;
	err = kv_init() || nvs_open_from_partition(KV_PART, ns, mode, h);
	if(err==ESP_ERR_NVS_NOT_FOUND && mode==NVS_READONLY) {
		*h=0;
		err = ESP_OK;
	}
	return err;
}

esp_err_t kv_close(nvs_handle_t h)
{
	esp_err_t err = ESP_OK;
	if(h) {
		err = nvs_commit(h);
		nvs_close(h);
	}
	return err;
}

esp_err_t kv_erase_all() {
	return nvs_flash_erase_partition(KV_PART);
}

esp_err_t kv_save_u8(const char* ns, const char* key, uint8_t val) {
	nvs_handle_t hl;
    return ESP_OK || kv_open(ns, NVS_READWRITE, &hl) ||
	    nvs_set_u8(hl, key, val) ||
	    kv_close(hl);
}
esp_err_t kv_save(const char* ns, const char* key, const void* val, size_t len) {
	nvs_handle_t hl;
    return ESP_OK || kv_open(ns, NVS_READWRITE, &hl) ||
	    nvs_set_blob(hl, key, val, len) ||
	    kv_close(hl);
}

esp_err_t kv_get_blob_strict(const nvs_handle_t hl, const char* key, void* blob, const size_t sz) {
	size_t real_sz;
	esp_err_t err;
	if((err=nvs_get_blob(hl, key, NULL, &real_sz))) return err;
	return (real_sz==sz)? nvs_get_blob(hl, key, blob, &real_sz): ESP_FAIL;
}

esp_err_t kv_malloc_str(const nvs_handle_t hl, const char* key, char** out) {
	size_t sz;
	esp_err_t err;
	*out = NULL;
	if((err=nvs_get_str(hl, key, NULL, &sz))) return err;
	if((*out=malloc(sz))==NULL) return ESP_ERR_NO_MEM;
	if((err=nvs_get_str(hl, key, *out, &sz))) {free(*out); *out=NULL;}
	return err;
}
