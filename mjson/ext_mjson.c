#include "ext_mjson.h"

// without unescaping
bool emjson_get_i32(const char* buf, int len, char* query, void* out) {
    double d;
    if(mjson_get_number(buf, len, query, &d)) {
        *(int32_t*)out = d;
        return true;
    } else {
        return false;
    }
}

bool emjson_get_i16(const char* buf, int len, char* query, void* out) {
    double d;
    if(mjson_get_number(buf, len, query, &d)) {
        *(int16_t*)out = d;
        return true;
    } else {
        return false;
    }
}

bool emjson_get_float(const char* buf, int len, char* query, float* f) {
    double d;
    if(mjson_get_number(buf, len, query, &d)) {
        *f = (float)d;
        return true;
    }
    return false;
}

bool emjson_get_i16_from_string(const char* buf, int len, char* query, int16_t* out) {
    int16_t temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char* endptr;
        temp = (int16_t) strtol(buf+1, &endptr, 10);
        if (endptr == buf+len-1) {
            *out = temp;
            return true;
        }
    }
    return false;
}

bool emjson_get_i8(const char* buf, int len, char* query, void* out) {
    double d;
    if(mjson_get_number(buf, len, query, &d)) {
        *(int8_t*)out = d;
        return true;
    } else {
        return false;
    }
}

bool emjson_get_i8_from_string(const char* buf, int len, char* query, int8_t* out) {
    int8_t temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char* endptr;
        temp = (int8_t) strtol(buf+1, &endptr, 10);
        if (endptr == buf+len-1) {
            *out = temp;
            return true;
        }
    }
    return false;
}

bool emjson_get_float_from_string_non_strict(const char* buf, int len, char* query, float* out) {
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        *out = (float) strtof(buf+1, NULL);
        return true;
    }
    return false;
}

bool emjson_get_float_from_string(const char* buf, int len, char* query, float* out) {
    float temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char* endptr;
        temp = (float) strtof(buf+1, &endptr);
        if (endptr == buf+len-1) {
            *out = temp;
            return true;
        }
    }
    return false;

}

bool emjson_get_hex_from_string(const char* buf, int len, char* query, uint32_t* out) {
    uint32_t temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char* endptr;
        temp = (uint32_t) strtol(buf+2, &endptr, 16);
        if (endptr == buf+len-1) {
            *out = temp;
            return true;
        }
    }
    return false;

}

bool emjson_get_string(const char* buf, int len, char* query, char* out, int n) {
    return mjson_get_string(buf, len, query, out, n) != -1;
}
bool emjson_locate_string(const char* buf, int len, char* query, const char** out, int* n) {
    if(mjson_find(buf, len, query, (const char**)out, n)!=MJSON_TOK_STRING) return false;
    (*out)++; *n -=2;
    return true;
}
char* emjson_find_string(const char* buf, int len, char* query) {
    const char* out; int n;
    if(mjson_find(buf, len, query, &out, &n)==MJSON_TOK_STRING) return out+1;
    return NULL;
}

void emjson_find_string_batch(const char* buf, int len, char* query, char** out, ...) {
    va_list args;
    va_start(args, out);
    while (query != NULL) {
        *out= emjson_find_string(buf, len, query);
        query = va_arg(args, char*);
        out = va_arg(args, char**);
    }
    va_end(args);
}
void emjson_truncate_string(char* s) {
    if(s) {
        char* p = strchr(s, '"');
        if(p) *p = 0;
    }
}
void emjson_truncate_string_batch(char* s, ...) {
    va_list args;
    va_start(args, s);
    while (s != NULL) {
        emjson_truncate_string(s);
        s = va_arg(args, char*);
    }
    va_end(args);
}
bool emjson_malloc_string(const char* buf, int len, char* query, char** out) {
    const char* a; int n;
    if(emjson_locate_string(buf, len, query, &a, &n)) {
        if((*out=malloc(n+1))) {
            memcpy(*out, a, n);
            (*out)[n]=0;
            return true;
        }
    }
    return false;
}

bool emjson_get_bool(const char* buf, int len, char* query, bool* b) {
    int i;
    if(mjson_get_bool(buf, len, query, &i)) {
        *b = i;
        return true;
    } else {
        return false;
    }
}

bool emjson_get_time_from_string(const char* buf, int len, char* query, emjson_time_t* time) {
    emjson_time_t temp = *time;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char* sepptr, *endptr;
        time->hr = (emjson_hr_t) strtol(buf+1, &sepptr, 10);
        time->min = (emjson_min_t) strtol(sepptr+1, &endptr, 10);
        if (endptr == buf+len-1)
            return true;
    }
    *time = temp;
    return false;
}

bool emjson_get_date_from_string(const char* buf, int len, char* query, emjson_date_t* date) {
    emjson_date_t temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char *endptr;
        buf += 1;
        temp.yr = (emjson_yr_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.mon = (emjson_mon_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.day = (emjson_day_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        *date = temp;
        return true;
    }
    return false;
}

bool emjson_get_datetime_from_string(const char* buf, int len, char* query, emjson_datetime_t* datetime) {
    emjson_datetime_t temp;
    if(mjson_find(buf, len, query, &buf, &len)==MJSON_TOK_STRING) {
        char *endptr;
        buf += 1;
        temp.date.yr = (emjson_yr_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.date.mon = (emjson_mon_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.date.day = (emjson_day_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.time.hr = (emjson_hr_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        buf = endptr+ 1;
        temp.time.min = (emjson_min_t) strtol(buf, &endptr, 10);
        if(buf == endptr) return false;
        *datetime = temp;
        return true;
    }
    return false;
}

void emjson_for_each(const char* buf, int len, emjson_for_each_cb_t cb, void* user_data) {
    int koff, klen, voff, vlen, vtype, off;
    for (off = 0; (off = mjson_next(buf, len, off, &koff, &klen, &voff, &vlen, &vtype)) != 0; ) {
        cb(buf, koff+1, klen-2, voff, vlen, user_data);
    }
}