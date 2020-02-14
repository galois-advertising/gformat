//solopointer1202@gmail.com
#include <string.h>
#include "checksum.h"
#include "log.h"

namespace galois::format {

template<typename header_t>
pb_buffer_writer<header_t>::pb_buffer_writer() : 
    _current_buf_position(0) {

}
template<typename header_t>
pb_buffer_writer<header_t>::~pb_buffer_writer() {

}

template<typename header_t>
int pb_buffer_writer<header_t>::write_init() {
    _current_buf_position = 0;
    return 0;
}

template<typename header_t>
int pb_buffer_writer<header_t>::get_write_len() const {
    return _current_buf_position;
}

template<typename header_t>
error_t pb_buffer_writer<header_t>::write_record(
    header_t& header, const void* body, uint32_t body_len,
    char* buffer, uint32_t buffer_len) {

    if (body == nullptr || buffer == nullptr) {
        FATAL("Parameter invalid [%p][%p].", body, buffer);
        return error_t::ERROR;
    }

    int ret = checksum(body, body_len, header.checksum);
    if (ret != 0) {
        FATAL("failed to checksum [%p][%p].", body, buffer);
        return error_t::ERROR;
    }

    header.magic_number = header_t::MAGIC_NUMBER;
    header.pack_len = body_len;

    char* start_pos = buffer + _current_buf_position;
    if (header.extend == 0) {
        if (_current_buf_position + header_t::BASE_HEADER_LENGTH + body_len > buffer_len) {
            DEBUG("cur buf pos [%u] + base header len [%u] + body len [%u] > buf len [%d].",
                _current_buf_position, header_t::BASE_HEADER_LENGTH,
                body_len, buffer_len);
            return error_t::BUFFER_TOO_SMALL;
        }
        memcpy(start_pos, &header, header_t::BASE_HEADER_LENGTH);
        memcpy(start_pos + header_t::BASE_HEADER_LENGTH, body, body_len);
        _current_buf_position += header_t::BASE_HEADER_LENGTH + body_len;
    } else {
        if (_current_buf_position + sizeof(header_t) + body_len > buffer_len) {
            DEBUG("cur buf pos [%u] + extend header len [%u] + body len [%u] > buf len [%d].",
                _current_buf_position, sizeof(header_t),
                body_len, buffer_len);
            return error_t::BUFFER_TOO_SMALL;
        }
        memcpy(start_pos, &header, sizeof(header_t));
        memcpy(start_pos + sizeof(header_t), body, body_len);
        _current_buf_position += sizeof(header_t) + body_len;
    }
    return error_t::SUCCESS;
}

}  
