//solopointer1202@gmail.com
#include <string.h>
#include "pack_header.h"
#include "log.h"

namespace galois::format {

uint16_t get_header_len(const pack_header_t& header) {
    return header.extend == 0 ? BASE_HEADER_LENGTH : EXTEND_HEADER_LENGTH;
}

error_t write_header(const pack_header_t& header, char* buffer, uint32_t buffer_len) {
    if (buffer == nullptr) {
        FATAL("buffer is nullptr.", "");
        return error_t::ERROR;
    }
    uint32_t header_len = get_header_len(header);
    if (buffer_len < header_len) {
        WARNING("buffer_len[%u] < header_len[%u].",
            buffer_len, header_len);
        return error_t::BUFFER_TOO_SMALL;
    } 
    memcpy(buffer, &header, header_len);
    return error_t::SUCCESS;
}

error_t write_header(const pack_header_t& header, FILE* fp) {
    if (fp == nullptr) {
        FATAL("fp is nullptr.", "");
        return error_t::ERROR;
    }
    uint32_t header_len = get_header_len(header);
    size_t write_len = fwrite(&header, 1, header_len, fp);    
    if (write_len != header_len) {
        FATAL("write_len[%lu] != header_len[%u]: %m.",
            write_len, header_len);
        return error_t::FILE_WRITE_FAILED;
    }
    return error_t::SUCCESS;
}

} 