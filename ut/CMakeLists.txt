cmake_minimum_required(VERSION 3.13)
project(mfrc522_ut CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

set(MAIN_DIR ${mfrc522_SOURCE_DIR})
include_directories(${MAIN_DIR}/include)

########################
### Test executables ###
########################
add_executable(TestMfrc522DrvCommon TestMfrc522DrvCommon.cpp common/TestCommon.cpp common/Mockable.cpp)
target_link_libraries(TestMfrc522DrvCommon gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvCommon mfrc522_src_ut)
target_link_options(TestMfrc522DrvCommon PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

add_executable(TestMfrc522DrvIrq TestMfrc522DrvIrq.cpp common/TestCommon.cpp common/Mockable.cpp)
target_link_libraries(TestMfrc522DrvIrq gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvIrq mfrc522_src_ut)
target_link_options(TestMfrc522DrvIrq PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

add_executable(TestMfrc522DrvLlPtr TestMfrc522DrvLlPtr.cpp)
target_link_libraries(TestMfrc522DrvLlPtr gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvLlPtr mfrc522_src_ll_ptr_ut)

add_executable(TestMfrc522DrvNoLlDelay TestMfrc522DrvNoLlDelay.cpp common/TestCommon.cpp common/Mockable.cpp)
target_link_libraries(TestMfrc522DrvNoLlDelay gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvNoLlDelay mfrc522_src_no_ll_delay_ut)
target_link_options(TestMfrc522DrvNoLlDelay PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

add_executable(TestMfrc522DrvTimer TestMfrc522DrvTimer.cpp common/TestCommon.cpp common/Mockable.cpp)
target_link_libraries(TestMfrc522DrvTimer gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvTimer mfrc522_src_ut)
target_link_options(TestMfrc522DrvTimer PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

add_executable(TestMfrc522Picc TestMfrc522Picc.cpp)
target_link_libraries(TestMfrc522Picc gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522Picc mfrc522_src_ut)
target_link_options(TestMfrc522Picc PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

add_executable(TestMfrc522DrvPiccActivities TestMfrc522DrvPiccActivities.cpp common/TestCommon.cpp common/Mockable.cpp)
target_link_libraries(TestMfrc522DrvPiccActivities gmock_main gmock gtest pthread)
target_link_libraries(TestMfrc522DrvPiccActivities mfrc522_src_ut)
target_link_options(TestMfrc522DrvPiccActivities PRIVATE "-rdynamic" "LINKER:--no-as-needed" "-ldl")

###################
### Test suites ###
###################
add_test(NAME TestMfrc522DrvCommon COMMAND TestMfrc522DrvCommon)
add_test(NAME TestMfrc522DrvIrq COMMAND TestMfrc522DrvIrq)
add_test(NAME TestMfrc522DrvLlPtr COMMAND TestMfrc522DrvLlPtr)
add_test(NAME TestMfrc522DrvNoLlDelay COMMAND TestMfrc522DrvNoLlDelay)
add_test(NAME TestMfrc522DrvTimer COMMAND TestMfrc522DrvTimer)
add_test(NAME TestMfrc522Picc COMMAND TestMfrc522Picc)
add_test(NAME TestMfrc522DrvPiccActivities COMMAND TestMfrc522DrvPiccActivities)