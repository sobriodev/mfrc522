cmake_minimum_required(VERSION 3.12)
project(mfrc522_ut CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

SET(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPPUTEST_DIR ${CURRENT_DIR}/lib/cpputest)
SET(MAIN_DIR ${mfrc522_SOURCE_DIR})

# Enable memory leaks detection for cpputest
SET(CMAKE_CXX_FLAGS "-include ${CPPUTEST_DIR}/include/CppUTest/MemoryLeakDetectorMallocMacros.h")

# Include directories
include_directories(${MAIN_DIR}/include)
include_directories(${CPPUTEST_DIR}/include)

# Library directories
link_directories(${CPPUTEST_DIR})

# Executables
add_executable(TestMfrc522Drv TestRunner.cpp TestMfrc522Drv.cpp)
target_link_libraries(TestMfrc522Drv mfrc522_src CppUTest CppUTestExt)

# Test suites
add_test(NAME TestMfrc522Drv COMMAND TestMfrc522Drv -v)