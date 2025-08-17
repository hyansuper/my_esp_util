#include "sol2lun.h"
#include <stdio.h>
#define YEAR_BASE 2023 // 2024 ~ 2034
const static uint8_t jq_base[] = {0x55, 0x33, 0x55, 0x44, 0x55, 0x55, 0x76, 0x77, 0x77, 0x88, 0x77, 0x66};
const static uint8_t jq_db[] = {0xfc,0xff,0xdb, 0x0d,0x08,0x00, 0x80,0x9a,0x42, 0xc4,0xba,0xc2, 0xfc,0xff,0xc3, 0x0d,0x08,0x00, 0x80,0x9a,0x42, 0xc4,0xba,0xc2, 0xfc,0xff,0xc3, 0x0d,0x08,0x00, 0x80,0x9a,0x42, 0xc4,0xba,0xc2};
const static uint8_t lun_db[] = {0x95,0xc8,0x56, 0x28,0x48,0x1b, 0x9c,0x95,0x3a, 0x2f,0x94,0x1c, 0x24,0x2c,0x19, 0x59,0x5d,0x32, 0x2b,0xac,0x32, 0x21,0x68,0x15, 0xd6,0x58,0x2b, 0x29,0xa4,0x2d, 0xde,0x4a,0x5d, 0x31,0x48,0x2d};

const char* LUN_TEN="初十廿";
const char* LUN_TEN_END="初二三";
const char* LUN_NUM="一二三四五六七八九十冬腊";
const char* JIEQI = "小寒大寒立春雨水惊蛰春分清明谷雨立夏小满芒种夏至小暑大暑立秋处暑白露秋分寒露霜降立冬小雪大雪冬至";

uint8_t _sol2jieqi_num(struct tm* dt) {
    int sec_half = dt->tm_mday > 15;
    const uint8_t base = jq_base[dt->tm_mon];
    if( dt->tm_mday ==

        ((jq_db[(dt->tm_year+1900 - YEAR_BASE) *3 + dt->tm_mon/4] & (1<<(dt->tm_mon %4 *2 +sec_half))) ? 1: 0)
        + (sec_half? ((base >>4) + 15) : (base & 0x0f))
    )
        return (dt->tm_mon*2 + sec_half);
    return -1;
}

int leap_year(int year){
    return ((year % 4 == 0) && (year % 100!= 0)) || (year%400 == 0);
}

lunar_date_t sol2lun(struct tm* dt) {
    int days = dt->tm_yday;
    int ind = (dt->tm_year+1900 - YEAR_BASE)*3;
    uint32_t rec = *((uint32_t*)&lun_db[ind]);
    if((rec & 0b111111) > days) {
        rec = *((uint32_t*)&lun_db[ind-3]);
        days += 365+ leap_year(dt->tm_year+1900-1);
    }
    days -= (rec & 0b111111) -1;
    int mon = 0;
    int mdays;
    while(days > (mdays = rec&(1<<(mon+10))?30:29)) {
        days -= mdays;
        mon ++;
    }
    int leap_mon = (rec>>6)&0xf;
    lunar_date_t ld = {.day=days-1};
    if(leap_mon) {
    // 比如2025年是闰六月，则六月过两次，第7个农历月才是闰六月。此时 leap_mon=6 (从1开始), mon=6 (从0开始)是第7个农历月
        if(leap_mon==mon)
            ld.leap = 1;
        if(leap_mon<=mon)
            mon --;
    }
    ld.mon = mon;
    ld.jieqi = _sol2jieqi_num(dt);
    return ld;
}

// char* jieqi_str(lunar_date_t* ld) {
//     return ld->jieqi==-1?"":&JIEQI[ld->jieqi*3];
// }

// char* lunar_str(lunar_date_t* ld) {
//     static char _str[16];
//     sprintf(_str, "%.3s%.3s月%.3s%.3s",
//                     ld->leap?"闰":"",
//                     ld->mon?(LUN_NUM+ld->mon*3):"正",
//                     (ld->day%10==9?LUN_TEN_END:LUN_TEN)+ld->day/10*3,
//                     ld->day%10==9?"十":(LUN_NUM+ld->day%10*3));
//     return _str;
// }
