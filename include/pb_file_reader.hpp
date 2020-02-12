//solopointer1202@gmail.com
#include <string.h>
#include <new>
#include "format.h"
#include "pack_header.h"
#include "pb_file_reader.h"
#include "calc_checksum.h"
#include "log.h"

namespace galois::format {

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
error_t pb_file_reader<header_t>::read_header(header_t& header, FILE* protobuf_fp) {
    if (auto res = read_basic_header(header, protobuf_fp); res != error_t::SUCCESS) {
        return res;
    }
    if (auto res = read_extend_segments(header, protobuf_fp); res != error_t::SUCCESS) {
        return res;
    }
    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_basic_header(header_t& header, FILE* protobuf_fp) {
    if (protobuf_fp == nullptr) {
        FATAL("protobuf_fp is nullptr.", "");
        return error_t::ERROR;
    }

    if ((_magic_pos = ftell(protobuf_fp)) == -1) {
        FATAL("failed to get magicpos.");
        return error_t::ERROR;
    }

    uint32_t magic_number_len = sizeof(MAGIC_NUMBER);
    uint32_t read_len = fread(&header, 1, magic_number_len, protobuf_fp);

    if (read_len < magic_number_len) {
        if (ferror(protobuf_fp) != 0) {
            FATAL("failed to read magic [%ld].", _magic_pos);
            return error_t::ERROR;
        }
        
        if (feof(protobuf_fp) != 0) {
            if (read_len > 0) {
                DEBUG("reach eof when read magic, [%u][%u][%ld].",
                    read_len, magic_number_len, _magic_pos);
                return error_t::DATA_INCOMPLETE;
            }
            DEBUG("reach eof.", "");
            return error_t::REACH_EOF;
        }
        FATAL("unkown error, magic pos is [%ld].", _magic_pos);
        return error_t::ERROR;
    }

    uint16_t magic_number = header.magic_number;
    if (magic_number != MAGIC_NUMBER) {
        if (_last_magic_check) {
            FATAL("magic != MAGIC [%x][%x][%ld].", magic_number, MAGIC_NUMBER, _magic_pos);
            _last_magic_check = false;
        }
        return error_t::MAGIC_ERROR;
    }
    _last_magic_check = true;
    uint32_t remain_len = BASE_HEADER_LENGTH - magic_number_len;
    read_len = fread(((char*)&header) + magic_number_len, 1, remain_len, protobuf_fp);
    if (read_len < remain_len) {
        if (ferror(protobuf_fp) != 0) {
            FATAL("failed to read base header [%ld].", _magic_pos);
            return error_t::ERROR;
        }
        
        if (feof(protobuf_fp) != 0) {
            DEBUG("reach eof when read base header[%d][%ld].", read_len, _magic_pos);
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
        FILE* protobuf_fp) {
    if (protobuf_fp == nullptr) {
        FATAL("protobuf_fp is nullptr.");
        return error_t::ERROR;
    }
    
    if (header.extend == 0) {
        return error_t::SUCCESS;
    }

    uint32_t remain_len = EXTEND_HEADER_LENGTH - BASE_HEADER_LENGTH;
    uint32_t read_len = fread(((char*)&header) + BASE_HEADER_LENGTH, 1, remain_len, protobuf_fp);
    if (read_len < remain_len) {
        if (ferror(protobuf_fp) != 0) {
            FATAL("failed to read extend[%ld].", _magic_pos);
            return error_t::ERROR;
        }
        if (feof(protobuf_fp) != 0) {
            DEBUG("reach eof when read extend[%d][%ld].", read_len, _magic_pos);
            return  error_t::DATA_INCOMPLETE;
        }
        FATAL("unkown error [%ld].", _magic_pos);
        return error_t::ERROR;
    }

    return error_t::SUCCESS;
}

template<typename header_t>
error_t pb_file_reader<header_t>::read_body(
        const header_t& header,
        void* body_buf,
        uint32_t body_buf_len,
        FILE* protobuf_fp) {
    if (protobuf_fp == nullptr) {
        FATAL("protobuf_fp is nullptr.");
        return error_t::ERROR;
    }

    if (body_buf == nullptr) {
        FATAL("body_buf is nullptr.");
        return error_t::ERROR;
    }

    uint32_t body_len = header.pack_len;
    if (body_len > body_buf_len) {
        FATAL("body buf len need enlarge[%u][%u][%ld].",
                body_buf_len, body_len, _magic_pos);
        return error_t::RECORD_TOO_BIG;
    }

    uint32_t read_len = fread(body_buf, 1u, body_len, protobuf_fp);
    if (read_len < body_len) {
        if (ferror(protobuf_fp) != 0) {
            FATAL("failed to read body[%ld].", _magic_pos);
            return error_t::ERROR;
        }
        if (feof(protobuf_fp) != 0) {
            DEBUG("reach eof when read body[%d][%ld].", read_len, _magic_pos);
            return error_t::DATA_INCOMPLETE;
        }
        FATAL("unkown error[%ld].", _magic_pos);
        return error_t::ERROR;
    }
    uint16_t body_sign = 0;
    int ret = calc_checksum(body_buf, body_len, body_sign);
    if (ret != 0) {
        FATAL("failed to compute crc.");
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
