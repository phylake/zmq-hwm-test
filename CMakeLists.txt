cmake_minimum_required (VERSION 3.0)
project (HWM_TEST, VERSION 0.0.1)

find_library(zmq, libzmq)
find_library(czmq, libczmq)

include_directories("${PROJECT_BINARY_DIR}/src")

add_executable(hwm_test src/main.c)
target_link_libraries (hwm_test czmq zmq)
