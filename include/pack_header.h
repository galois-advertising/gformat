//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
#include <stdio.h>
#include "format.h"

namespace galois::format {

const uint16_t MAGIC_NUMBER = 0xf83f;
// no virtual method
struct pack_header_t {
    static const uint16_t MAGIC_NUMBER = 0xf83f;
    static const uint16_t BASE_HEADER_LENGTH = 12;
    typedef uint16_t magic_t;
    magic_t magic_number;
    uint16_t checksum;

    uint32_t pack_len;
    uint32_t source : 8;
    uint32_t level : 10;
    uint32_t reserve1 : 13;
    uint32_t extend : 1;

    uint32_t heart_beat : 1;
    uint32_t op_type : 2;
    uint32_t change_flag : 1;
    uint32_t reserve2 : 28;
    uint64_t time_stamp;
    uint64_t eventid;
    uint64_t key_id;
    uint64_t reserve3;

    pack_header_t() : 
            magic_number(MAGIC_NUMBER),
            checksum(0),
            pack_len(0),
            source(0),
            level(0),
            reserve1(0),
            extend(0),
            heart_beat(0),
            op_type(0),
            change_flag(0),
            reserve2(0),
            time_stamp(0),
            eventid(0),
            key_id(0),
            reserve3(0) {}
};

const uint16_t BASE_HEADER_LENGTH = 12;

uint16_t get_header_len(const pack_header_t& header);
error_t write_header(const pack_header_t& header, char* buffer, uint32_t buffer_len);
error_t write_header(const pack_header_t& header, FILE* fp);

}  