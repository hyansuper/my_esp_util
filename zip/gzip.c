#include "zip.h"

#define FTEXT 1
#define FHCRC 2
#define FEXTRA 4
#define FNAME 8
#define FCOMMENT 16

/*
for data header check, return err code, dst_len will store decompressed length on return
see https://github.com/jibsen/tinf/
*/
int gunzip_check(const unsigned char* src, const unsigned int src_len, unsigned char** zip, unsigned int* ziplen, unsigned int* dst_len, unsigned int* crc32) {

	const unsigned char flg = src[3];

	if(src_len<18 				/* Check room for at least 10 byte header and 8 byte trailer */
		|| src[0]!=0x1f || src[1]!=0x8b /* Check id bytes */
		|| src[2]!=8 					/* Check method is deflate */
		|| flg&0xe0 					/* Check that reserved bits are zero */
	) {
		return ZIP_ERR_FORMAT;
	}

	/* Skip base header of 10 bytes */
	unsigned char* p = src + 10;
	const unsigned char* const end = src + src_len;

	/* Skip extra data if present */
	if ( (flg&FEXTRA) && (p+= 2+read_le16(p))> end ) {
		return ZIP_ERR_FORMAT;
	}

	/* Skip file name if present */
	if (flg & FNAME) {
		do {
			if(p >= end) return ZIP_ERR_FORMAT;
		} while(*p ++);
	}

	/* Skip file comment if present */
	if (flg & FCOMMENT) {
		do {
			if(p >= end) return ZIP_ERR_FORMAT;
		} while(*p ++);
	}

	/* Check header crc if present */
	if ( (flg&FHCRC) && (
						#ifdef CONFIG_ZIP_GUNZIP_CHECK_HEADER_CRC32
						(read_le16(p) != (calc_crc32(src, p-src) & 0x0000FFFF)) ||
						#endif
						((p+=2) > end))
	) {
		return ZIP_ERR_FORMAT;
	}

	// get decompressed length
	if (end-p < 8) {
		return ZIP_ERR_FORMAT;
	}

	*dst_len = read_le32(end-4);
	*crc32 = read_le32(end-8);
	*zip = p;
	*ziplen = end-p-8;

	return ZIP_OK;
}