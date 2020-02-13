
#include <string.h>
#include <new>
#include "pack_header.h"
#include "pb_fixed_file_reader.h"
#include "checksum.h"
#include "log.h"

namespace galois::format {

template<typename header_t>
pb_fixed_file_reader<header_t>::pb_fixed_file_reader() {}

template<typename header_t>
pb_fixed_file_reader<header_t>::~pb_fixed_file_reader() {}

template<typename header_t>
error_t pb_fixed_file_reader<header_t>::read_record(
        header_t& header, void* body_buf,
        uint32_t body_buf_len, FILE* protobuf_fp) {
    if (body_buf == nullptr || body_buf_len == 0 || protobuf_fp == nullptr) {
        FATAL("Parameter invalid [%p][%u][%p].", body_buf, body_buf_len, protobuf_fp);
        return error_t::ERROR;
    }
    auto status = error_t::SUCCESS;
    while (true) {
        status = this->read_header(header, protobuf_fp);
        if (status == error_t::REACH_EOF) {
            return status;
        }
        if (status == error_t::MAGIC_ERROR || status == error_t::DATA_INCOMPLETE) {
            if (fseek(protobuf_fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("failed to seek magic pos[%ld].", this->magic_number_pos() + 1);
                return error_t::ERROR;
            }
            continue;
        } else if (status == error_t::SUCCESS) {
            DEBUG("read header[%ld].", this->magic_number_pos());
        } else {
            FATAL("failed to read header.", "");
            return error_t::ERROR;
        }
        status = this->read_body(header, body_buf, body_buf_len, protobuf_fp);
        if (status == error_t::SUCCESS) {
            DEBUG("read header [%ld].", this->magic_number_pos());
            break;
        } else if (status == error_t::CHECK_ERROR
                || status == error_t::RECORD_TOO_BIG
                || status == error_t::DATA_INCOMPLETE) {
            if (fseek(protobuf_fp, this->magic_number_pos() + 1, SEEK_SET) == -1) {
                FATAL("failed to seek magic pos[%ld].", this->magic_number_pos() + 1);
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
