//solopointer1202@gmail.com
#pragma once
#include "format.h"

namespace galois::format {

template<typename header_t>
class pb_buffer_reader {
public:
    pb_buffer_reader();
    ~pb_buffer_reader();
    int read_init();
    error_t read_record(header_t& header, char** body_buf,
        const char* buffer, uint32_t buffer_len);
    error_t read_record_strong(header_t& header, char** body_buf,
        const char* buffer, uint32_t buffer_len);
protected:
    error_t read_header(header_t& header, const char* buffer, uint32_t buffer_len);
    error_t read_body(header_t& header, char** body_buf, const char* buffer, uint32_t buffer_len);
private:
    uint32_t _magic_pos;
    uint32_t _current_buf_position;
    bool _last_magic_check;
};

}  

#include "pb_buffer_reader.hpp"

