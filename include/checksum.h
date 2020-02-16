//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
namespace galois::gformat {
int checksum(const void* key, int length, uint16_t& check_sum);
}  