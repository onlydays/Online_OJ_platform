cmake_minimum_required(VERSION 3.16)
project(oj_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

file(GLOB_RECURSE BACKEND_SOURCES
    src/*.cpp
    src/*.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

find_package(mysql-connector-cpp REQUIRED)

add_executable(oj_backend ${BACKEND_SOURCES})

target_link_libraries(oj_backend PRIVATE mysqlcppconn)
