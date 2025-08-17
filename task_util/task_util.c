#include "task_util.h"
#include "esp_memory_utils.h"
#include "esp_log.h"

void capped_task_delete(TaskHandle_t task) {
    uint8_t *task_stack = pxTaskGetStackStart(task);
    if(task_stack == NULL) return;
    if (esp_ptr_internal(task_stack) == false) {
        vTaskDeleteWithCaps(task);
    } else {
        vTaskDelete(task);
    }
}

esp_err_t capped_task_create(TaskHandle_t* task_hd, const char* name, void(task_fn)(void*), void* arg, capped_task_config_t* conf) {
    TaskHandle_t t;
    BaseType_t r;
    if (conf->caps) {
        r = xTaskCreatePinnedToCoreWithCaps(task_fn, name, conf->stack, arg, conf->prio, &t, conf->core, conf->caps);
    } else {
        r = xTaskCreatePinnedToCore(task_fn, name, conf->stack, arg, conf->prio, &t, conf->core);
    }
    if (r==pdPASS) {
        *task_hd = t;
        return ESP_OK;
    }
    return ESP_ERR_NO_MEM;
}