
#include "helper.h"
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


namespace base {
namespace helper {

class md5ctx {
public:
    struct arg_list *c;

    int sawline;

    // Crypto variables blanked after summing
    unsigned int state[5];
    unsigned int oldstate[5];
    uint64_t count;
    union {
        char c[64];
        unsigned int i[16];
    } buffer;
};


// for(i=0; i<64; i++) md5table[i] = abs(sin(i+1))*(1<<32);  But calculating
// that involves not just floating point but pulling in -lm (and arguing with
// C about whether 1<<32 is a valid thing to do on 32 bit platforms) so:

static uint32_t md5table[64] = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
  0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
  0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
  0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
  0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
  0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static const uint8_t md5rot[64] = {
  7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
  5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
static void md5_transform(md5ctx& TT)
{
    unsigned x[4], *b = TT.buffer.i;
    int i;

    memcpy(x, TT.state, sizeof(x));

    for (i=0; i<64; i++) {
        unsigned int in, temp, swap;
        if (i<16) {
            in = i;
            temp = x[1];
            temp = (temp & x[2]) | ((~temp) & x[3]);
        } else if (i<32) {
            in = (1+(5*i))&15;
            temp = x[3];
            temp = (x[1] & temp) | (x[2] & ~temp);
        } else if (i<48) {
            in = (3*i+5)&15;
            temp = x[1] ^ x[2] ^ x[3];
        } else {
            in = (7*i)&15;
            temp = x[2] ^ (x[1] | ~x[3]);
        }
        temp += x[0] + b[in] + md5table[i];
        swap = x[3];
        x[3] = x[2];
        x[2] = x[1];
        x[1] += rol(temp, md5rot[i]);
        x[0] = swap;
    }
    for (i=0; i<4; i++) TT.state[i] += x[i];
}


// Fill the 64-byte working buffer and call transform() when full.

static void hash_update(char *data, unsigned int len, md5ctx& TT)
{
    unsigned int i, j;

    j = TT.count & 63;
    TT.count += len;

    for (;;) {
        // Grab next chunk of data, return if it's not enough to process a frame
        i = 64 - j;
        if (i>len) i = len;
        memcpy(TT.buffer.c+j, data, i);
        if (j+i != 64) break;

#if 0
        // Process a frame
        if (IS_BIG_ENDIAN)
            for (j=0; j<16; j++) TT.buffer.i[j] = SWAP_LE32(TT.buffer.i[j]);
#endif
        md5_transform(TT);
        j=0;
        data += i;
        len -= i;
    }
}


static void do_hash(int fd, md5ctx& TT)
{
    uint64_t count;
    int i;
    char buf;

    /* SHA1 initialization constants  (md5sum uses first 4) */
    TT.state[0] = 0x67452301;
    TT.state[1] = 0xEFCDAB89;
    TT.state[2] = 0x98BADCFE;
    TT.state[3] = 0x10325476;
    TT.state[4] = 0xC3D2E1F0;
    TT.count = 0;

    char buffer[4096];
    for (;;) {
        i = read(fd, buffer, sizeof(buffer));
        if (i < 1) break;
        hash_update(buffer, i, TT);
    }

    count = TT.count << 3;

    // End the message by appending a "1" bit to the data, ending with the
    // message size (in bits, big endian), and adding enough zero bits in
    // between to pad to the end of the next 64-byte frame.
    //
    // Since our input up to now has been in whole bytes, we can deal with
    // bytes here too.

    buf = 0x80;
    do {
        hash_update(&buf, 1, TT);
        buf = 0;
    } while ((TT.count & 63) != 56);
    hash_update((char *)&count, 8, TT);

    /*
    if (sha1)
        for (i = 0; i < 20; i++)
            sprintf(toybuf+2*i, "%02x", 255&(TT.state[i>>2] >> ((3-(i & 3)) * 8)));
    else for (i=0; i<4; i++) sprintf(toybuf+8*i, "%08x", bswap_32(TT.state[i]));

    // Wipe variables. Cryptographer paranoia.
    memset(TT.state, 0, sizeof(TT)-((long)TT.state-(long)&TT));
    i = strlen(toybuf)+1;
    memset(toybuf+i, 0, sizeof(toybuf)-i);
    */
}

#define BSWAP32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

std::string md5sum(const std::string& file)
{
    md5ctx ctx;
    int fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
        return "";
    }
    do_hash(fd, ctx);
    close(fd);

    char buffer[128] = {0};
    for (int i = 0; i < 4; i++) {
        sprintf(buffer+ 8 * i, "%08x", BSWAP32(ctx.state[i]));
    }
    return buffer;
}

} // namespace helper
} // namespace base

#ifdef __UNITTEST_MD5__
int main(int argc, char * argv[])
{
    std::string md5 = base::helper::md5sum(argv[1]);
    printf("%s  %s\n", md5.c_str(), argv[1]);
    return 0;
}

#endif

