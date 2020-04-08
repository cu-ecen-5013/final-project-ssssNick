cmake_minimum_required(VERSION 3.0.0)
project(unity-test)
add_subdirectory(Unity)
include_directories(Unity/src .)
add_executable(unity-test test/Test_circular_buffer.c test/test_runners/Test_circular_buffer_Runner.c aesd-circular-buffer.c)
target_link_libraries(unity-test unity)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
# See https://cmake.org/cmake/help/latest/command/add_custom_command.html
# Generate our test runner using the auto_generate.sh script
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runners/Test_circular_buffer_Runner.c
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test/auto_generate.sh
                )
set(CMAKE_BUILD_TYPE Debug)
