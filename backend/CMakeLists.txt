cmake_minimum_required(VERSION 3.16)
project(oj_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

find_package(OpenSSL REQUIRED)

find_path(MYSQL_INCLUDE_DIR mysql_driver.h PATHS /usr/include)
find_library(MYSQLCPPCONN_LIB mysqlcppconn PATHS /usr/lib/x86_64-linux-gnu)

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQLCPPCONN_LIB)
    message(FATAL_ERROR "mysql-connector-cpp not found")
endif()

include_directories(${MYSQL_INCLUDE_DIR})

file(GLOB_RECURSE BACKEND_SOURCES
    src/*.cpp
    src/*.cc
)

add_executable(oj_backend ${BACKEND_SOURCES})

target_link_libraries(oj_backend PRIVATE
    ${MYSQLCPPCONN_LIB}
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
