cmake_minimum_required(VERSION 3.12)
project(mfrc522_ut CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

SET(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPPUTEST_DIR ${CURRENT_DIR}/lib/cpputest)
SET(MAIN_DIR ${mfrc522_SOURCE_DIR})

# Enable memory leaks detection for cpputest
SET(CMAKE_CXX_FLAGS "-include ${CPPUTEST_DIR}/include/CppUTest/MemoryLeakDetectorMallocMacros.h")

# Include directories
include_directories(${MAIN_DIR}/include)
include_directories(${CPPUTEST_DIR}/include)
include_directories(stub)

# Library directories
link_directories(${CPPUTEST_DIR})

# Executables
add_executable(TestMfrc522DrvCommon TestRunner.cpp TestMfrc522DrvCommon.cpp stub/Mfrc522FakeDev.cpp stub/StubCommon.cpp)
target_link_libraries(TestMfrc522DrvCommon mfrc522_src CppUTest CppUTestExt)

add_executable(TestMfrc522DrvLlPtr TestRunner.cpp TestMfrc522DrvLlPtr.cpp)
target_link_libraries(TestMfrc522DrvLlPtr mfrc522_src CppUTest CppUTestExt)

add_executable(TestMfrc522DrvNoLlDelay TestRunner.cpp TestMfrc522DrvNoLlDelay.cpp mock/MockCommon.cpp)
target_link_libraries(TestMfrc522DrvNoLlDelay mfrc522_src_no_ll_delay CppUTest CppUTestExt)

# Test suites
add_test(NAME TestMfrc522DrvCommon COMMAND TestMfrc522DrvCommon -v)
add_test(NAME TestMfrc522DrvLlPtr COMMAND TestMfrc522DrvLlPtr -v)
add_test(NAME TestMfrc522DrvNoLlDelay COMMAND TestMfrc522DrvNoLlDelay -v)