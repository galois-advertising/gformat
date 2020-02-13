#include "checksum.h"
#include "log.h"

namespace galois::format {

int checksum(const void* key, int length, uint16_t& check_sum) {
    if (key == nullptr) {
        FATAL("key is null.", "");
        return -1;
    }
    uint32_t murmur_sum;
    const unsigned int m = 0x5bd1e995;
    const unsigned int seed = (0xdeadbeef * length);
    const int r = 24;

    unsigned int h = seed ^ length;
    const unsigned char* data = (const unsigned char*)key;

    while (length >= 4) {
        unsigned int k = *(unsigned int*)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        length -= 4;
    }
    switch (length) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
    }


    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    murmur_sum = h;

    check_sum = (murmur_sum + (murmur_sum >> 16)) & 0xffff;
    return 0;
}

}  
