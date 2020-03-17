//solopointer1202@gmail.com
#include <string.h>
#include <new>
#include "checksum.h"
#include "log.h"

namespace galois::gformat {

template<typename header_t>
pb_file_reader<header_t>::pb_file_reader() : 
    _magic_pos(0), _last_magic_check(true) {

}

template<typename header_t>
pb_file_reader<header_t>::~pb_file_reader() {

}

template<typename header_t>
int64_t pb_file_reader<header_t>::magic_number_pos() const {
    return _magic_pos;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_header(header_t& header, FILE* fp) {
    if (auto res = read_basic_header(header, fp); res != error_t::SUCCESS) {
        return res;
    }
    if (auto res = read_extend_segments(header, fp); res != error_t::SUCCESS) {
        return res;
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_basic_header(header_t& header, FILE* fp) {
    if (fp == nullptr) {
        FATAL("fp is nullptr.", "");
        return error_t::ERROR;
    }

    if ((_magic_pos = ftell(fp)) == -1) {
        FATAL("failed to get magicpos.", "");
        return error_t::ERROR;
    }

    uint32_t read_len = fread(&header, 1, sizeof(typename header_t::magic_t), fp);

    if (read_len < sizeof(typename header_t::magic_t)) {
        if (ferror(fp) != 0) {
            FATAL("failed to read magic [%ld].", _magic_pos);
            return error_t::ERROR;
        }
        
        if (feof(fp) != 0) {
            if (read_len > 0) {
                TRACE("Reach EOF [%u][%u][%ld].",
                    read_len, sizeof(typename header_t::magic_t), _magic_pos);
                return error_t::DATA_INCOMPLETE;
            }
            TRACE("Reach EOF.", "");
            return error_t::REACH_EOF;
        }
        FATAL("Something fails [%ld].", _magic_pos);
        return error_t::ERROR;
    }

    uint16_t magic_number = header.magic_number;
    if (magic_number != header_t::MAGIC_NUMBER) {
        if (_last_magic_check) {
            FATAL("magic != MAGIC [%x][%x][%ld].", magic_number, header_t::MAGIC_NUMBER, _magic_pos);
            _last_magic_check = false;
        }
        return error_t::MAGIC_ERROR;
    }
    _last_magic_check = true;
    uint32_t remain_len = header_t::BASE_HEADER_LENGTH - sizeof(typename header_t::magic_t);
    read_len = fread(((char*)&header) + sizeof(typename header_t::magic_t), 1, remain_len, fp);
    if (read_len < remain_len) {
        if (ferror(fp) != 0) {
            FATAL("failed to read base header [%ld].", _magic_pos);
            return error_t::ERROR;
        }
        
        if (feof(fp) != 0) {
            TRACE("Reach EOF when read base header[%d][%ld].", read_len, _magic_pos);
            return error_t::DATA_INCOMPLETE;
        }
        FATAL("unkown error [%ld].", _magic_pos);
        return error_t::ERROR;
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_extend_segments(
        header_t& header,
        FILE* fp) {
    if (fp == nullptr) {
        FATAL("fp is nullptr.", "");
        return error_t::ERROR;
    }
    
    if (header.extend == 0) {
        return error_t::SUCCESS;
    }

    uint32_t remain_len = sizeof(header_t) - header_t::BASE_HEADER_LENGTH;
    uint32_t read_len = fread(((char*)&header) + header_t::BASE_HEADER_LENGTH, 1, remain_len, fp);
    if (read_len < remain_len) {
        if (ferror(fp) != 0) {
            FATAL("failed to read extend[%ld].", _magic_pos);
            return error_t::ERROR;
        }
        if (feof(fp) != 0) {
            TRACE("reach eof when read extend[%d][%ld].", read_len, _magic_pos);
            return  error_t::DATA_INCOMPLETE;
        }
        FATAL("unkown error [%ld].", _magic_pos);
        return error_t::ERROR;
    }

    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_body(
    const header_t& header, void* body_buf, 
    uint32_t body_buf_len, FILE* fp) {
    if (fp == nullptr) {
        FATAL("fp is nullptr.", "");
        return error_t::ERROR;
    }

    if (body_buf == nullptr) {
        FATAL("body_buf is nullptr.", "");
        return error_t::ERROR;
    }

    uint32_t body_len = header.pack_len;
    if (body_len > body_buf_len) {
        FATAL("body buf len need enlarge[%u][%u][%ld].",
                body_buf_len, body_len, _magic_pos);
        return error_t::RECORD_TOO_BIG;
    }

    uint32_t read_len = fread(body_buf, 1u, body_len, fp);
    if (read_len < body_len) {
        if (ferror(fp) != 0) {
            FATAL("failed to read body[%ld].", _magic_pos);
            return error_t::ERROR;
        }
        if (feof(fp) != 0) {
            TRACE("reach eof when read body[%d][%ld].", read_len, _magic_pos);
            return error_t::DATA_INCOMPLETE;
        }
        FATAL("unkown error[%ld].", _magic_pos);
        return error_t::ERROR;
    }
    uint16_t body_sign = 0;
    int ret = checksum(body_buf, body_len, body_sign);
    if (ret != 0) {
        FATAL("failed to checksum.", "");
        return error_t::ERROR;
    }
    if (body_sign != header.checksum) {
        FATAL("failed to check sign[%x][%x][%ld].",
                header.checksum, body_sign, _magic_pos);
        return error_t::CHECK_ERROR;
    }
    return error_t::SUCCESS;
}

}  
