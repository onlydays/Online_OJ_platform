cmake_minimum_required(VERSION 3.16)

enable_testing()

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/backend/src)
include_directories(${CMAKE_SOURCE_DIR}/backend/third_party)

set(UNIT_TEST_SOURCES
    unit/test_config.cc
    unit/test_logger.cc
    unit/test_connection_pool.cc
    unit/test_json.cc
    unit/test_auth.cc
    unit/test_user_model.cc
    unit/test_problem_model.cc
    unit/test_testcase_model.cc
    unit/test_judge_result.cc
    unit/test_thread_pool.cc
    unit/test_submission_model.cc
    ${CMAKE_SOURCE_DIR}/backend/src/utils/config.cc
    ${CMAKE_SOURCE_DIR}/backend/src/utils/logger.cc
    ${CMAKE_SOURCE_DIR}/backend/src/db/connection_pool.cc
    ${CMAKE_SOURCE_DIR}/backend/src/utils/json.cpp
    ${CMAKE_SOURCE_DIR}/backend/src/auth.cc
    ${CMAKE_SOURCE_DIR}/backend/src/models/user.cc
    ${CMAKE_SOURCE_DIR}/backend/src/models/problem.cc
    ${CMAKE_SOURCE_DIR}/backend/src/models/testcase.cc
    ${CMAKE_SOURCE_DIR}/backend/src/models/submission.cc
    ${CMAKE_SOURCE_DIR}/backend/src/judge/judge_result.cc
    ${CMAKE_SOURCE_DIR}/backend/src/judge/judge_manager.cc
)

add_executable(unit_tests ${UNIT_TEST_SOURCES})

target_link_libraries(unit_tests PRIVATE
    GTest::GTest
    GTest::Main
    mysqlcppconn
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

target_compile_options(unit_tests PRIVATE -Wall -Wextra)

add_test(NAME UnitTests COMMAND unit_tests)
