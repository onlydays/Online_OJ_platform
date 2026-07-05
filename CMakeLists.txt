cmake_minimum_required(VERSION 3.16)
project(OnlineOJPlatform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_BACKEND "Build backend" ON)
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_BACKEND)
    add_subdirectory(backend)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
