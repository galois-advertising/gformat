//solopointer1202@gmail.com
#pragma once 

namespace galois::format {
enum class error_t {
    SUCCESS, NEED_CONTINUE, BUFFER_TOO_SMALL, MAGIC_ERROR, 
    RECORD_TOO_BIG, CHECK_ERROR, DATA_INCOMPLETE,
    ERROR, REACH_EOF, FILE_WRITE_FAILED
};
}  
