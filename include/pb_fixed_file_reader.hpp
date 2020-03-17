//solopointer1202@gmail.com
#include <string.h>
#include <new>
#include "pack_header.h"
#include "pb_fixed_file_reader.h"
#include "checksum.h"
#include "log.h"

namespace galois::gformat {

template<typename header_t>
pb_fixed_file_reader<header_t>::pb_fixed_file_reader() {}

template<typename header_t>
pb_fixed_file_reader<header_t>::~pb_fixed_file_reader() {}

template<typename header_t>
error_t pb_fixed_file_reader<header_t>::read_record(
    header_t& header, void* body_buf, uint32_t body_buf_len, FILE* fp) {
    if (body_buf == nullptr || body_buf_len == 0 || fp == nullptr) {
        FATAL("Parameter invalid [%p][%u][%p].", body_buf, body_buf_len, fp);
        return error_t::ERROR;
    }
    auto status = error_t::SUCCESS;
    while (true) {
        status = this->read_header(header, fp);
        if (status == error_t::REACH_EOF) {
            return status;
        }
        if (status == error_t::MAGIC_ERROR || status == error_t::DATA_INCOMPLETE) {
            if (fseek(fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("failed to seek magic pos[%ld].", this->magic_number_pos() + 1);
                return error_t::ERROR;
            }
            continue;
        } else if (status == error_t::SUCCESS) {
            TRACE("Invoke read_header successfully[%ld].", this->magic_number_pos());
        } else {
            FATAL("Failed to read_header.", "");
            return error_t::ERROR;
        }
        status = this->read_body(header, body_buf, body_buf_len, fp);
        if (status == error_t::SUCCESS) {
            TRACE("Invoke read_body successfully [%ld].", this->magic_number_pos());
            break;
        } else if (status == error_t::CHECK_ERROR
                || status == error_t::RECORD_TOO_BIG
                || status == error_t::DATA_INCOMPLETE) {
            if (fseek(fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("Failed to seek magic pos[%ld].", this->magic_number_pos() + 1);
                return error_t::ERROR;
            }
            continue;
        } else {
            FATAL("failed to read body.", "");
            return error_t::ERROR;
        }
    }
    return error_t::SUCCESS;
}

}  
