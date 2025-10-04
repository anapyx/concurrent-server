cmake_minimum_required(VERSION 3.16)
project(ConcurrentServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Common source files
set(COMMON_SOURCES
    src/logger.cpp
    src/cli.cpp
)

# Main server executable (integrates logging tests and HTTP server)
add_executable(concurrent-server
    src/main.cpp
    src/http_server.cpp
    src/http_handler.cpp
    src/thread_pool.cpp
    src/connection_queue.cpp
    ${COMMON_SOURCES}
)

target_link_libraries(concurrent-server Threads::Threads)

# Test client
add_executable(test-client
    src/test_client.cpp
)

# Load test client
add_executable(load-test
    src/load_test.cpp
)

# Link socket libraries on Windows
if(WIN32)
    target_link_libraries(concurrent-server ws2_32)
    target_link_libraries(test-client ws2_32)
    target_link_libraries(load-test ws2_32)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)