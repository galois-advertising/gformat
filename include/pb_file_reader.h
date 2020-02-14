//solopointer1202@gmail.com
#pragma once
#include <stdint.h>
#include "format.h"

namespace galois::format {

template<typename header_t>
class pb_file_reader {
public:
    pb_file_reader();
    virtual ~pb_file_reader();
    virtual error_t read_record(header_t& header, void* body_buf, uint32_t body_buf_len, FILE* fp) = 0;
protected:
    virtual error_t read_header(header_t& header, FILE* fp);
    virtual error_t read_basic_header(header_t& header, FILE* fp);
    virtual error_t read_extend_segments(header_t& header, FILE* fp);
    virtual error_t read_body(const header_t& header, void* body_buf, uint32_t body_buf_len, FILE* fp);
    int64_t magic_number_pos() const;
private:
    int64_t _magic_pos;
    bool _last_magic_check;
};

}  

#include "pb_file_reader.hpp"

