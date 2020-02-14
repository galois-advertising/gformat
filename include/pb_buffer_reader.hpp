//solopointer1202@gmail.com
#include <string.h>
#include <new>
#include "checksum.h"
#include "log.h"

namespace galois::format {

template<typename header_t>
pb_buffer_reader<header_t>::pb_buffer_reader() :
    _magic_pos(0), _current_buf_position(0), _last_magic_check(true) {
}

template<typename header_t>
pb_buffer_reader<header_t>::~pb_buffer_reader() {

}

template<typename header_t>
int pb_buffer_reader<header_t>::read_init() {
    _magic_pos = 0;
    _current_buf_position = 0;
    _last_magic_check = true;
    return 0;
}

template<typename header_t>
error_t pb_buffer_reader<header_t>::read_header(
    header_t& header, const char* buffer, uint32_t buffer_len) {
    if (buffer == nullptr) {
        FATAL("Parameter invalid[%p].", buffer);
        return error_t::ERROR;
    }

    _magic_pos = _current_buf_position;

    if (_current_buf_position == buffer_len) {
        DEBUG("Reach EOF.", "");
        return error_t::REACH_EOF;
    } else if (_current_buf_position + sizeof(typename header_t::magic_t) > buffer_len) {
        FATAL("Reach end [%u][%u][%u].",
                buffer_len - _current_buf_position, sizeof(typename header_t::magic_t), _magic_pos);
        return error_t::DATA_INCOMPLETE;
    } else {
        //DEBUG("read header[%u].", _magic_pos);
    }

    const char* header_pos = buffer + _current_buf_position;
    const header_t& read_header = *((const header_t*)header_pos); 
    _current_buf_position += sizeof(typename header_t::magic_t);

    uint16_t magic_number = read_header.magic_number;
    if (magic_number != header_t::MAGIC_NUMBER) {
        if (_last_magic_check) {
            FATAL("magic number != MAGIC NUMBER [%x][%x][%u].",
                magic_number, header_t::MAGIC_NUMBER, _magic_pos);
            _last_magic_check = false;
        }
        return error_t::MAGIC_ERROR;
    }
    _last_magic_check = true;

    uint32_t remain_len = header_t::BASE_HEADER_LENGTH - sizeof(typename header_t::magic_t);
    if (_current_buf_position + remain_len <= buffer_len) {
        memcpy((char*)&header, header_pos, header_t::BASE_HEADER_LENGTH);
        _current_buf_position += remain_len;
    } else {
        FATAL("reach EOF [%u][%u].",
                buffer_len - _current_buf_position, _magic_pos);
        return error_t::DATA_INCOMPLETE;
    }

    if (read_header.extend == 1) {
        remain_len = sizeof(header_t) - header_t::BASE_HEADER_LENGTH;
        if (_current_buf_position + remain_len <= buffer_len) {
            memcpy((char*)&header + header_t::BASE_HEADER_LENGTH, buffer + _current_buf_position, remain_len);
            _current_buf_position += remain_len;
        } else {
            FATAL("Reach EOF [%u][%u].",
                    buffer_len - _current_buf_position, _magic_pos);
            return error_t::NEED_CONTINUE;
        }
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_buffer_reader<header_t>::read_body(
    header_t& header, char** body_buf,
    const char* buffer, uint32_t buffer_len) {
    if (body_buf == nullptr || buffer == nullptr) {
        FATAL("Parameter invalid [%p][%p].", body_buf, buffer);
        return error_t::ERROR;
    }

    uint32_t body_len = header.pack_len;
    if (body_len > buffer_len - _current_buf_position) {
        FATAL("invalid body len[%u][%u][%u].",
            body_len, buffer_len - _current_buf_position, _magic_pos);
        return error_t::NEED_CONTINUE;
    }

    *body_buf = const_cast<char*>(buffer) + _current_buf_position;
    _current_buf_position += body_len;

    uint16_t body_sign = 0;
    int ret = checksum((void*)(*body_buf), body_len, body_sign);
    if (ret != 0) {
        FATAL("failed to checksum[%p][%u].", *body_buf, body_len);
        return error_t::ERROR;
    }

    if (body_sign != header.checksum) {
        FATAL("failed to check sign [%x][%x][%u].",
            header.checksum, body_sign, _magic_pos);
        return error_t::CHECK_ERROR;
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_buffer_reader<header_t>::read_record(
    header_t& header, char** body_buf,
    const char* buffer, uint32_t buffer_len) {
    if (body_buf == nullptr || buffer == nullptr) {
        FATAL("Parameter invalid[%p][%p].", body_buf, buffer);
        return error_t::ERROR;
    }

    auto read_status = error_t::SUCCESS;

    while (true) {
        read_status = read_header(header, buffer, buffer_len);
        if (read_status == error_t::SUCCESS) {
            DEBUG("Invoke read_header successfully [%u].", _magic_pos);
        } else if (read_status == error_t::REACH_EOF) {
            return error_t::REACH_EOF;
        } else if (read_status == error_t::MAGIC_ERROR
                || read_status == error_t::NEED_CONTINUE) {
            _current_buf_position = _magic_pos + 1;
            continue;
        } else {
            FATAL("Failed to read_header.", "");
            return error_t::ERROR;
        }

        read_status = read_body(header, body_buf, buffer, buffer_len);
        if (read_status == error_t::SUCCESS) {
            DEBUG("Invoke read_body successfully [%u].", _magic_pos);
            break;
        } else if (read_status == error_t::NEED_CONTINUE
                || read_status == error_t::CHECK_ERROR) {
            _current_buf_position = _magic_pos + 1;
            continue;
        } else {
            FATAL("Failed to read_body.", "");
            return error_t::ERROR;
        }
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_buffer_reader<header_t>::read_record_strong(
    header_t& header, char** body_buf,
    const char* buffer, uint32_t buffer_len) {
    if (body_buf == nullptr || buffer == nullptr) {
        FATAL("Parameter invalid[%p][%p].", body_buf, buffer);
        return error_t::ERROR;
    }

    auto read_status = error_t::SUCCESS;

    while (true) {
        read_status = read_header(header, buffer, buffer_len);
        if (read_status == error_t::SUCCESS) {
            DEBUG("read header[%u].", _magic_pos);
        } else if (read_status == error_t::REACH_EOF) {
            return error_t::REACH_EOF;
        } else if (read_status == error_t::MAGIC_ERROR
                || read_status == error_t::NEED_CONTINUE) {
            FATAL("failed to read header[%d]", read_status);
            return error_t::ERROR;
        } else {
            FATAL("failed to read header.", "");
            return error_t::ERROR;
        }

        read_status = read_body(header, body_buf, buffer, buffer_len);
        if (read_status == error_t::SUCCESS) {
            DEBUG("read header[%u].", _magic_pos);
            break;
        } else if (read_status == error_t::NEED_CONTINUE
                || read_status == error_t::CHECK_ERROR) {
            FATAL("failed to read body[%d]", read_status);
            return error_t::ERROR;
        } else {
            FATAL("failed to read body.", "");
            return error_t::ERROR;
        }
    }
    return error_t::SUCCESS;
}
}  
