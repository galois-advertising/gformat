//solopointer1202@gmail.com
#include <string>
#include <new>
#include "log.h"

namespace galois::gformat {

template<typename header_t>
pb_writable_file_reader<header_t>::pb_writable_file_reader() {
}

template<typename header_t>
pb_writable_file_reader<header_t>::~pb_writable_file_reader() {
}

template<typename header_t>
error_t pb_writable_file_reader<header_t>::read_record(
    header_t& header, void* body_buf, uint32_t body_buf_len, FILE* fp) {
    if (body_buf == nullptr || body_buf_len == 0 || fp == nullptr) {
        FATAL("Parameter invalid [%p][%u][%p].", body_buf, body_buf_len, fp);
        return error_t::ERROR;
    }
    error_t read_status = error_t::SUCCESS;
    while (true) {
        read_status = this->read_header(header, fp);
        if (read_status == error_t::REACH_EOF || read_status == error_t::DATA_INCOMPLETE) {
            return read_status;
        }
        
        if (read_status == error_t::MAGIC_ERROR) {
            if (fseek(fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("Failed to seek magic number pos,[%ld].",
                        this->magic_number_pos() + 1);
                return error_t::ERROR;
            }
            continue;
        } else if (read_status == error_t::SUCCESS) {
            DEBUG("Read header [%ld].", this->magic_number_pos());
        } else {
            FATAL("Failed to read header.", "");
            return error_t::ERROR;
        }

        read_status = this->read_body(header, body_buf, body_buf_len, fp);
        if (read_status == error_t::SUCCESS) {
            DEBUG("Read header[%ld].", this->magic_number_pos());
            break;
        } else if (read_status == error_t::DATA_INCOMPLETE) {
            return read_status;
        } else if (read_status == error_t::CHECK_ERROR || read_status == error_t::RECORD_TOO_BIG) {
            if (fseek(fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("Failed to seek magic number pos[%ld].",
                    this->magic_number_pos() + 1);
                return error_t::ERROR;
            }
            continue;
        } else {
            FATAL("Failed to read body.", "");
            return error_t::ERROR;
        }
    }
    return error_t::SUCCESS;
}

}  
