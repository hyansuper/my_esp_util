# https://www.hko.gov.hk/tc/gts/time/calendar/text/files/T{YEAR}c.txt
from pathlib import Path
path = Path(__file__).parent

import re
def split(s):
    return re.split(' +', s)

def get_mon(s):
    return int(s[s.index('年')+1:s.index('月')])

def get_day(s):
    return int(s[s.index('月')+1:s.index('日')])

def mon2i(m): # eg. mon2i('十二月') / mon2i('正月')
    i='正二三四五六七八九十'.index(m[0]) +1
    if len(m)==3:
        i+= '一二'.index(m[1]) +1
    return i

jq_base = [
    5,20,
    3,18,
    5,20,

    4,19,
    5,20,
    5,20,

    6,22,
    7,22,
    7,22,

    8,23,
    7,22,
    6,21,
]

jq_bits = 1
d = mon_index = 0
start, end = 2023, 2034

jq_arr=[]
lun_arr=[]
for yy in range(start, end+2):
    with open(path / f'files/T{yy}c.txt','r',encoding='big5') as f:
        f.readline()
        f.readline()
        f.readline()
        jq = 0
        for line in f:
            col = split(line)
            solar_day=get_day(col[0])
            solar_mon=get_mon(col[0])
            if '月' in col[1]:
                if last_col[1]=='三十':
                    d |= 1<<(mon_index+10)
                mon_index += 1
                if col[1]=='正月':
                    if start<= (yy-1) <= end:
                        lun_arr.append(d)
                    mon_index = d = 0
                    d |= solar_day -1 + (31 if solar_mon==2 else 0)
                elif col[1][0]=='閏':
                    d |= mon2i(col[1][1:]) << 6

            last_col = col

            if len(col)==5:
                jq_ind = (solar_mon-1) *2 + (0 if solar_day<=15 else 1)
                ofs = solar_day - jq_base[jq_ind]
                if 0> ofs or ofs> (2**jq_bits)-1:
                    print(col)
                    raise 'jq_bits too small'
                jq |= ofs << (jq_ind*jq_bits)

        if start<= yy <= end:
            jq_arr.append(jq)


print(start, '-', end)
print('lun:')
for i in lun_arr:
    print(f'0x{i&0xff:02x},0x{(i&0xff00)>>8:02x},0x{(i&0xff0000)>>16:02x}', end=', ')
print('\njq:')
for i in jq_arr:
    print(f'0x{i&0xff:02x},0x{(i&0xff00)>>8:02x},0x{(i&0xff0000)>>16:02x}', end=', ')
print('\njq_base:')
for i, j in zip(jq_base[0::2], jq_base[1::2]):
    k = i|(j-15)<<4
    print(f'0x{k:x}', end=', ')

