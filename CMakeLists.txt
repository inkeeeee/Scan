cmake_minimum_required(VERSION 3.28)
project(scan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(target scan)

find_package(GTest REQUIRED)
add_library(${target} INTERFACE)

target_include_directories(${target} INTERFACE include/)

file(GLOB TEST_SRC_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(unit_tests "${TEST_SRC_FILES}")
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unit_tests
    PRIVATE
        ${PROJECT_NAME}
        GTest::GTest
        GTest::Main)

enable_testing()
add_test(NAME Tests COMMAND unit_tests)
