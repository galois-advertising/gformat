project(gformat)
cmake_minimum_required(VERSION 2.8)

find_package(Boost REQUIRED COMPONENTS log)
if(NOT Boost_FOUND)
    message("Can not find Boost")
endif()
message(STATUS "Found Boost IncPath ${Boost_INCLUDE_DIRS}")
message(STATUS "Found Boost LibPath ${Boost_LIBRARY_DIRS}")
message(STATUS "Found Boost LIBRARIES ${Boost_LIBRARIES}")

add_definitions(-DBOOST_SPIRIT_THREADSAFE)
add_definitions(-DBOOST_ALL_DYN_LINK)
add_definitions(-D_DEBUG)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -std=c++17 -fno-strict-aliasing -Wno-register")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -std=c++17 -fno-strict-aliasing -Wno-register")
set(CMAKE_BUILD_TYPE "Release")

include_directories("${CMAKE_SOURCE_DIR}/common/util/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

ADD_SUBDIRECTORY(common)


set(SRCS
"${PROJECT_SOURCE_DIR}/src/checksum.cpp" 
"${PROJECT_SOURCE_DIR}/src/pack_header.cpp" 
)

set(TEST_SRCS
"${PROJECT_SOURCE_DIR}/test/main.cpp" 
)

add_library(gformat STATIC ${SRCS})

add_executable(gformat_test ${TEST_SRCS})
target_link_libraries(gformat_test gformat common boost_log-mt)
