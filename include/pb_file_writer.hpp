//solopointer1202@gmail.com
#include "checksum.h"
#include "log.h"

namespace galois::gformat {

template<typename header_t>
pb_file_writer<header_t>::pb_file_writer() {}

template<typename header_t>
pb_file_writer<header_t>::~pb_file_writer() {}

template<typename header_t>
error_t pb_file_writer<header_t>::write_header(
    header_t& header, const void* body_buf,
    uint32_t body_len, FILE* fp) {
    if (body_buf == nullptr) {
        FATAL("body buf is [%p].", body_buf);
        return error_t::ERROR;
    }

    if (fp == nullptr) {
        FATAL("fp is nullptr.", "");
        return error_t::ERROR;
    }

    int ret = checksum(body_buf, body_len, header.checksum);
    if (ret != 0) {
        FATAL("failed to checksum.", "");
        return error_t::ERROR;
    }

    header.magic_number = header_t::MAGIC_NUMBER;
    header.pack_len = body_len;

    uint32_t write_len = fwrite(&header, 1, header_t::BASE_HEADER_LENGTH, fp);
    if (write_len != header_t::BASE_HEADER_LENGTH) {
        FATAL("failed to write base header[%d].", write_len);
        return error_t::ERROR;
    }

    if (header.extend == 1) {
        write_len = fwrite(
            ((char*)&header) + header_t::BASE_HEADER_LENGTH, 1,
            sizeof(header_t) - header_t::BASE_HEADER_LENGTH, fp);
        if (write_len != sizeof(header_t) - header_t::BASE_HEADER_LENGTH) {
            FATAL("failed to write extend segments[%d].", write_len);
            return error_t::ERROR;
        }
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_writer<header_t>::write_record(
        header_t& header,
        const void* body_buf,
        uint32_t body_len,
        FILE* fp) {
    if (body_buf == nullptr || fp == nullptr) {
        FATAL("Parameter invalid [%p][%p].", body_buf, fp);
        return error_t::ERROR;
    }

    if (write_header(header, body_buf, body_len, fp) != error_t::SUCCESS) {
        FATAL("failed to set header.", "");
        return error_t::ERROR;
    }

    uint32_t write_len = fwrite(body_buf, 1, body_len, fp);
    if (write_len != body_len) {
        FATAL("failed to write body [%d].", write_len);
        return error_t::ERROR;
    }
    return error_t::SUCCESS;
}

}  
