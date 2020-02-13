#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include "log.h"
#include "pb_buffer_reader.h"
#include "pb_file_writer.h"

using namespace galois::format;

int test_read_write(std::string infile_name, std::string outfile_name)
{
    std::shared_ptr<FILE> infile(fopen(infile_name.c_str(), "r"), [](FILE* f){if(f)fclose(f);});
    if (infile == nullptr) {
        FATAL("open file[%s] failed.", infile_name.c_str());
        return -1;
    }
    std::shared_ptr<FILE> outfile(fopen(outfile_name.c_str(), "r"), [](FILE* f){if(f)fclose(f);});
    if (infile == nullptr) {
        FATAL("open file[%s] failed.", infile_name.c_str());
        return -1;
    }

    std::shared_ptr<char> buffer(new(std::nothrow) char[1024 * 1024], 
        [](char* p){if(p)delete[]p;});
    if (buffer == nullptr) {
        FATAL("malloc buffer failed.", "");
        return -1;
    }

    int read_len = fread(buffer.get(), 1, 1024 * 1024, infile.get());
    if (ferror(infile.get()) != 0) {
        FATAL("read from infile failed.", "");
        return -1;
    }

    WARNING("read %d bytes from infile.", read_len);

    pb_buffer_reader<pack_header_t> protobuf_reader;
    pb_file_writer<pack_header_t> protobuf_writer;
    pack_header_t header;

    char* body_pos = nullptr;
    int ret = 0;
    while (1) {
        auto rc = protobuf_reader.read_record(header, &body_pos, 
            buffer.get(), read_len);
        if (rc != error_t::SUCCESS) {
            WARNING("read one record failed.", "");
            ret = -1;
            break;
        } else if (rc == error_t::REACH_EOF) {
            WARNING("read over.", "");
            ret = 0;
            break;
        } else {
            std::cout<<"magic_number:"<<header.magic_number<<std::endl;
            std::cout<<"checksum:"<<header.checksum<<std::endl;
            std::cout<<"pack_len:"<<header.pack_len<<std::endl;
            std::cout<<"source:"<<header.source<<std::endl;
            std::cout<<"level:"<<header.level<<std::endl;
            std::cout<<"reserve1:"<<header.reserve1<<std::endl;
            std::cout<<"extend:"<<header.extend<<std::endl;
            std::cout<<"heart_beat:"<<header.heart_beat<<std::endl;
            std::cout<<"op_type:"<<header.op_type<<std::endl;
            std::cout<<"change_flag:"<<header.change_flag<<std::endl;
            std::cout<<"reserve2:"<<header.reserve2<<std::endl;
            std::cout<<"time_stamp:"<<header.time_stamp<<std::endl;
            std::cout<<"eventid:"<<header.eventid<<std::endl;
            std::cout<<"key_id:"<<header.key_id<<std::endl;
            std::cout<<"reserve3:"<<header.reserve3<<std::endl;
            if (protobuf_writer.write_record(header, body_pos, header.pack_len, outfile.get()) != error_t::SUCCESS) {
                FATAL("write one record to file failed.", "");
                ret = -1;
                break;
            }
        }
    }
    return ret;
}


int main(int argc, char ** argv) {
    if (argc != 3) {
        FATAL("%s inputfile outputfile", argv[0]);
        return 0;
    }
    std::cout<<argv[1]<<std::endl;
    std::cout<<argv[2]<<std::endl;
    test_read_write(argv[1], argv[2]);
}