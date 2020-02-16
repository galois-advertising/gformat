//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
#include "pb_file_reader.h"

namespace galois::gformat {

template<typename header_t>
class pb_writable_file_reader : public pb_file_reader<header_t> {
public:
    pb_writable_file_reader() {}
    virtual ~pb_writable_file_reader() {}
    virtual error_t read_record(header_t& header, void* body_buf,
        uint32_t body_buf_len, FILE* fp);
};

}  

#include "pb_writable_file_reader.hpp"

