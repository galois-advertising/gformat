//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
#include "format.h"

namespace galois::gformat {

template<typename header_t>
class pb_file_writer {
public:
    pb_file_writer();
    ~pb_file_writer();
    error_t write_record(header_t& header, const void* body_buf, uint32_t body_len, FILE* fp);
protected:
    error_t write_header(header_t& header, const void* body_buf, uint32_t body_len, FILE* fp);
};

}  

#include "pb_file_writer.hpp"
