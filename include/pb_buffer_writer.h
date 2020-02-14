//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
#include "format.h"

namespace galois::format {

template<typename header_t>
class pb_buffer_writer {
public:
    pb_buffer_writer();
    ~pb_buffer_writer();
    int write_init();
    int get_write_len() const;
    error_t write_record(header_t& header, const void* body, uint32_t body_len,
        char* buffer, uint32_t buffer_len);
private:
    uint32_t _current_buf_position;
};

}  

#include "pb_buffer_writer.hpp"

