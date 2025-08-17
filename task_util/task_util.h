#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RELEASE(p) do{free(p);p=NULL;}while(0)
#define RELEASE_TASK(task) do{if(task){capped_task_delete(task);task=NULL;}}while(0)

typedef struct {
    int stack;             /*!< Size of the task stack */
    int prio;              /*!< Priority of the task */
    int core;       /* eg. tskNO_AFFINITY */
    uint32_t caps;  /* eg. MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT */
} capped_task_config_t;

#define CAPPED_TASK_CONFIG_SIMPLE(_stack, _prio) {.stack=_stack, .prio=_prio, .core=tskNO_AFFINITY, .caps=0}
#define CAPPED_TASK_CONFIG_SPI(_stack, _prio) {.stack=_stack, .prio=_prio, .core=tskNO_AFFINITY, .caps=MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT}
#define CAPPED_TASK_CONFIG_SPI_CORE(_stack, _prio, _core) {.stack=_stack, .prio=_prio, .core=_core, .caps=MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT}

// NOTE: calling this funciton with NULL is to delete current task !
void capped_task_delete(TaskHandle_t task);
esp_err_t capped_task_create(TaskHandle_t* task_hd, const char* name, void(task_fn)(void*), void* arg, capped_task_config_t* conf);