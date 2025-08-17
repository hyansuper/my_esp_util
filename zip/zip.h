#pragma once

#define ZIP_OK 0
#define ZIP_ERR_FORMAT -1
#define ZIP_ERR_CRC -2
#define ZIP_ERR_NO_MEM -4

#include "puff/puff.h"
#include "tinf/tinf_common.h"
int gunzip_check(const unsigned char* src, const unsigned int src_len, unsigned char** zip, unsigned int* ziplen, unsigned int* dst_len, unsigned int* crc32);