cmake_minimum_required(VERSION 3.16)
project(LibTSLog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(PART1_SOURCES
    src/main.cpp
    src/logger.cpp
)

add_executable(libtslog_test ${PART1_SOURCES})

target_link_libraries(libtslog_test Threads::Threads)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)