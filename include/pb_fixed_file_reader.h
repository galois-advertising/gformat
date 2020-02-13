#pragma once
#include <stdint.h>
#include "pb_file_reader.h"

namespace galois::format {

template<typename header_t>
class pb_fixed_file_reader : public pb_file_reader<header_t> {
public:
    pb_fixed_file_reader();
    virtual ~pb_fixed_file_reader();
    virtual error_t read_record(header_t& header, void* body_buf, 
        uint32_t body_buf_len, FILE* protobuf_fp);
};

}  

#include "pb_fixed_file_reader.hpp"
