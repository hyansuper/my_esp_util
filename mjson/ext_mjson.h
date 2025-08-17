#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "mjson.h"

// quote ended string equal
#define QESTREQL(a,B) strncmp(a,B"\"",sizeof(B))==0


typedef uint8_t emjson_hr_t;
typedef uint8_t emjson_min_t;
typedef uint16_t emjson_yr_t;
typedef uint8_t emjson_mon_t;
typedef uint8_t emjson_day_t;

typedef struct {
    uint8_t hr;
    uint8_t min;
} emjson_time_t;

typedef struct {
    uint16_t yr;
    uint8_t mon;
    uint8_t day;
} emjson_date_t;

typedef struct {
    emjson_date_t date;
    emjson_time_t time;
} emjson_datetime_t;

// without unescaping
bool emjson_get_i32(const char* buf, int len, char* query, void* out);
bool emjson_get_i16(const char* buf, int len, char* query, void* out);
bool emjson_get_i8(const char* buf, int len, char* query, void* out);
bool emjson_get_i8_from_string(const char* buf, int len, char* query, int8_t* out);
bool emjson_get_i16_from_string(const char* buf, int len, char* query, int16_t* out);
bool emjson_get_float_from_string(const char* buf, int len, char* query, float* out);
bool emjson_get_float_from_string_non_strict(const char* buf, int len, char* query, float* out);
bool emjson_get_float(const char* buf, int len, char* query, float* f);
bool emjson_get_hex_from_string(const char* buf, int len, char* query, uint32_t* out);

bool emjson_get_string(const char* buf, int len, char* query, char* out, int n);

bool emjson_locate_string(const char* buf, int len, char* query, const char** out, int* n); // no copy
bool emjson_malloc_string(const char* buf, int len, char* query, char** out); // malloc then copy to out
char* emjson_find_string(const char* buf, int len, char* query);// locate str val
void emjson_find_string_batch(const char* buf, int len, char* query, char** out, ...);
void emjson_truncate_string(char* s); // truncate in-place by inserting NULL
void emjson_truncate_string_batch(char* s, ...);

bool emjson_get_time_from_string(const char* buf, int len, char* query, emjson_time_t* time);

bool emjson_get_date_from_string(const char* buf, int len, char* query, emjson_date_t* date);

bool emjson_get_datetime_from_string(const char* buf, int len, char* query, emjson_datetime_t* datetime);

// this is a safer version of mjson_get_bool(...int* i), coz sizeof(int)!=sizeof(bool)
bool emjson_get_bool(const char* buf, int len, char* query, bool* b);

typedef void (*emjson_for_each_cb_t)(const char* buf, int koff, int klen, int voff, int vlen, void* user_data);
void emjson_for_each(const char* buf, int len, emjson_for_each_cb_t cb, void* user_data);