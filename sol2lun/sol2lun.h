#pragma once
#include <time.h>
#include <stdint.h>

extern const char* LUN_TEN;
extern const char* LUN_TEN_END;
extern const char* LUN_NUM;
extern const char* JIEQI;

typedef struct {
    int8_t mon; // 0 based
    int8_t day; // 0 based
    int8_t leap; // 闰月？1：0
    int8_t jieqi; // 0 based nth节气in the year, -1 if not
} lunar_date_t;

lunar_date_t sol2lun(struct tm* dt);

#define FM_LUNAR "%.3s%.3s月%.3s%.3s"
#define PR_LUNAR(ld) \
                    ld.leap?"闰":"", \
                    ld.mon?(LUN_NUM+ld.mon*3):"正", \
                    (ld.day%10==9?LUN_TEN_END:LUN_TEN)+ld.day/10*3, \
                    ld.day%10==9?"十":(LUN_NUM+ld.day%10*3)

#define FM_JIEQI "%.6s"
#define PR_JIEQI(ld) \
                    ld.jieqi==-1?"":&JIEQI[ld.jieqi*6]

// char* jieqi_str(lunar_date_t* ld);

// char* lunar_str(lunar_date_t* ld);

int leap_year(int y);