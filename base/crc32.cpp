/* cksum.c - produce crc32 checksum value for each input
 *
 * Copyright 2008 Rob Landley <rob@landley.net>
 *
 * See http://opengroup.org/onlinepubs/9699919799/utilities/cksum.html
*/

#include "helper.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace base {
namespace helper {

static unsigned int cksum_be(unsigned int crc, unsigned char c, unsigned int crc_table[256])
{
    return (crc<<8)^crc_table[(crc>>24)^c];
}

static unsigned int cksum_le(unsigned int crc, unsigned char c, unsigned int crc_table[256])
{
    return crc_table[(crc^c)&0xff] ^ (crc>>8);
}

static unsigned int do_cksum(int fd, unsigned int crc_table[256], int little_endian)
{
    unsigned int crc = 0xffffffff;
    uint64_t llen = 0;
    unsigned int (*cksum)(unsigned int crc, unsigned char c, unsigned int crc_table[256]);

    cksum = little_endian  ? cksum_le : cksum_be;
    // CRC the data

    for (;;) {
        int len, i;

        char buf[1024];
        len = read(fd, buf, sizeof(buf));
        if (len < 0) {
            return 0;
        }
        if (len < 1) {
            break;
        }

        llen += len;
        for (i = 0; i < len; i++) {
            crc = cksum(crc, buf[i], crc_table);
        }
    }

    // CRC the length

    while (llen) {
        crc = cksum(crc, llen, crc_table);
        llen >>= 8;
    }
    return crc;
}

static unsigned int do_cksum(const char * buf, int len, unsigned int crc_table[256], int little_endian)
{
    unsigned int crc = 0xffffffff;
    unsigned int (*cksum)(unsigned int crc, unsigned char c, unsigned int crc_table[256]);

    cksum = little_endian  ? cksum_le : cksum_be;
    // CRC the data

    for (int i = 0; i < len; i++) {
        crc = cksum(crc, buf[i], crc_table);
    }

    // CRC the length

    while (len) {
        crc = cksum(crc, len, crc_table);
        len >>= 8;
    }
    return crc;
}

static void crc_init(unsigned int *crc_table, int little_endian)
{
    unsigned int i;

    // Init the CRC32 table (big endian)
    for (i = 0; i < 256; i++) {
        unsigned int j, c = little_endian ? i : i<<24;
        for (j = 8; j; j--) {
            if (little_endian) {
                c = (c&1) ? (c>>1)^0xEDB88320 : c>>1;
            } else {
                c = c&0x80000000 ? (c<<1)^0x04c11db7 : (c<<1);
            }
        }
        crc_table[i] = c;
    }
}


unsigned int crc32(const char * filename)
{
    unsigned int crc_table[256];
    crc_init(crc_table, 1);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    unsigned int crc = do_cksum(fd, crc_table, 1);
    close(fd);

    return crc;
}

unsigned int crc32(const void * buffer, int len)
{
    unsigned int crc_table[256];
    crc_init(crc_table, 1);
    unsigned int crc = do_cksum((const char *)buffer, len, crc_table, 1);
    return crc;
}

} // namespace helper
} // namespace base

#ifdef __UNITTEST_CRC32__
#include <stdio.h>
int main(int argc, char * argv[])
{
    if (argc < 2) {
        return -1;
    }
    unsigned int crc = base::helper::crc32(argv[1]);
    printf("%x\n", crc);
    return 0;
}
#endif

