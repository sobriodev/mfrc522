cmake_minimum_required(VERSION 3.13)
project(mfrc522_src C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")
option(BUILD_WITH_COVERAGE "Build the library with coverage results" OFF)
option(BUILD_FOR_UT "Build the library for unit testing" OFF)

include_directories(${mfrc522_SOURCE_DIR}/include)

if(BUILD_WITH_COVERAGE)
    # Fix gcov linking errors
    link_libraries(gcov)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
endif()

# Library versions for unit testing
if(BUILD_FOR_UT)
    set(LIB_INSTALL_DIR "/usr/local/lib/mfrc522")

    # Basic version of the library
    add_library(mfrc522_src_ut SHARED mfrc522_drv.c mfrc522_ll_stub.c)
    target_compile_definitions(mfrc522_src_ut PUBLIC MFRC522_LL_DEF MFRC522_LL_DELAY)

    # Build without low-level delay disabled
    add_library(mfrc522_src_no_ll_delay_ut SHARED mfrc522_drv.c mfrc522_ll_stub.c)
    target_compile_definitions(mfrc522_src_no_ll_delay_ut PUBLIC MFRC522_LL_DEF)

    # Build without low-level with 'pointer' low-level calls
    add_library(mfrc522_src_ll_ptr_ut SHARED mfrc522_drv.c)
    target_compile_definitions(mfrc522_src_ll_ptr_ut PUBLIC MFRC522_LL_PTR MFRC522_LL_DELAY)

    install(TARGETS mfrc522_src_ut mfrc522_src_no_ll_delay_ut mfrc522_src_ll_ptr_ut DESTINATION ${LIB_INSTALL_DIR})
endif()