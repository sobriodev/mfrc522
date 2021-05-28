cmake_minimum_required(VERSION 3.12)
project(mfrc522 C)
enable_testing()
include(CTest)

set(CMAKE_C_STANDARD 99)

add_subdirectory(src)
add_subdirectory(ut)