cmake_minimum_required(VERSION 3.16)

enable_testing()

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/backend/src)
include_directories(${CMAKE_SOURCE_DIR}/backend/third_party)

set(UNIT_TEST_SOURCES
    unit/test_config.cc
    unit/test_logger.cc
    ${CMAKE_SOURCE_DIR}/backend/src/utils/config.cc
    ${CMAKE_SOURCE_DIR}/backend/src/utils/logger.cc
)

add_executable(unit_tests ${UNIT_TEST_SOURCES})

target_link_libraries(unit_tests PRIVATE
    GTest::GTest
    GTest::Main
)

target_compile_options(unit_tests PRIVATE -Wall -Wextra)

add_test(NAME UnitTests COMMAND unit_tests)
