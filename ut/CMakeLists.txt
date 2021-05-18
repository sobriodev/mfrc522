cmake_minimum_required(VERSION 3.12)
project(mfrc522_ut CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

set(MAIN_DIR ${mfrc522_SOURCE_DIR})
include_directories(${MAIN_DIR}/include)
include_directories(lib/cutie)

# Cutie
set(CUTIE_DIR lib/cutie)
include(${CUTIE_DIR}/Cutie.cmake)
include_directories(lib/cutie)

# Test suites
add_cutie_test_target(TEST TestMfrc522DrvCommon.cpp SOURCES TestCommon.cpp)
target_link_libraries(TestMfrc522DrvCommon mfrc522_src)

add_cutie_test_target(TEST TestMfrc522DrvIrq.cpp SOURCES TestCommon.cpp)
target_link_libraries(TestMfrc522DrvIrq mfrc522_src)

add_cutie_test_target(TEST TestMfrc522DrvLlPtr.cpp)
target_link_libraries(TestMfrc522DrvLlPtr mfrc522_src_ll_ptr)

add_cutie_test_target(TEST TestMfrc522DrvNoLlDelay.cpp SOURCES TestCommon.cpp)
target_link_libraries(TestMfrc522DrvNoLlDelay mfrc522_src_no_ll_delay)

add_cutie_test_target(TEST TestMfrc522DrvTimer.cpp SOURCES TestCommon.cpp)
target_link_libraries(TestMfrc522DrvTimer mfrc522_src)

add_cutie_all_tests_target()