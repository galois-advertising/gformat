#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include "log.h"
#include "pb_buffer_reader.h"
#include "pb_fixed_file_reader.h"

using namespace galois::gformat;

int test_read_buffer(std::string infile_name, std::string outfile_name)
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
    pack_header_t header;

    char* body_pos = nullptr;
    int ret = 0;
    while (1) {
        auto rc = protobuf_reader.read_record(header, &body_pos, 
            buffer.get(), read_len);
        if (rc == error_t::REACH_EOF) {
            WARNING("read over.", "");
            ret = 0;
            break;
        } else if (rc == error_t::SUCCESS) {
            std::cout<<"magic_number:"<<header.magic_number
                <<" checksum:"<<header.checksum
                <<" pack_len:"<<header.pack_len
                <<" level:"<<header.level
                <<" extend:"<<header.extend
                <<" op_type:"<<header.op_type
                <<" time_stamp:"<<header.time_stamp
                <<" eventid:"<<header.eventid<<std::endl;
        } else {
            ERROR("Error:[%d]", static_cast<int>(rc));
            break;
        }
    }
    return ret;
}

int test_read_file(std::string infile_name, std::string outfile_name)
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

    const size_t BUF_SIZE = 1024 * 1024;
    std::shared_ptr<char> buffer(new(std::nothrow) char[BUF_SIZE], 
        [](char* p){if(p)delete[]p;});
    if (buffer == nullptr) {
        FATAL("malloc buffer failed.", "");
        return -1;
    }


    pb_fixed_file_reader<pack_header_t> protobuf_reader;
    pack_header_t header;

    char* body_pos = nullptr;
    int ret = 0;
    while (1) {
        auto rc = protobuf_reader.read_record(header, buffer.get(), BUF_SIZE, infile.get());
        if (rc == error_t::REACH_EOF) {
            WARNING("Reach EOF.", "");
            ret = 0;
            break;
        } else if (rc == error_t::SUCCESS) {
            std::cout<<"magic_number:"<<header.magic_number
                <<" checksum:"<<header.checksum
                <<" pack_len:"<<header.pack_len
                <<" level:"<<header.level
                <<" extend:"<<header.extend
                <<" op_type:"<<header.op_type
                <<" time_stamp:"<<header.time_stamp
                <<" eventid:"<<header.eventid<<std::endl;
        } else {
            ERROR("Error:[%d]", static_cast<int>(rc));
            break;
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
    //test_read_buffer(argv[1], argv[2]);
    test_read_file(argv[1], argv[2]);
}