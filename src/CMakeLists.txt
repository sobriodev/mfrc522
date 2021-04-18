cmake_minimum_required(VERSION 3.12)
project(mfrc522_src C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
option(BUILD_WITH_COVERAGE "Build the library with coverage results" OFF)
option(BUILD_FOR_UT "Build the library for unit testing" ON)

if(BUILD_WITH_COVERAGE)
    # Fix gcov linking errors
    link_libraries(gcov)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

include_directories(${mfrc522_SOURCE_DIR}/include)

add_library(mfrc522_src mfrc522_drv.c)
target_compile_definitions(mfrc522_src PUBLIC MFRC522_LL_PTR MFRC522_LL_DELAY)

# Library versions for unit testing
if(BUILD_FOR_UT)
    # Build without low-level delay enabled
    add_library(mfrc522_src_no_ll_delay mfrc522_drv.c)
    target_compile_definitions(mfrc522_src_no_ll_delay PUBLIC MFRC522_LL_DEF)
endif()