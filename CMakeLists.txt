cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.0)

project(opencl-work)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/module")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/libary)
include_directories(./include)
include_directories(thirdparty)
include_directories(./)
include_directories(./dlfcn)


add_library(
     cl-framwork
     thirdparty/clelapsedtimer.cpp
     thirdparty/clr2.cpp
     thirdparty/fdoutbuf.cpp
    thirdparty/ig.cpp
)

add_library(dlfcn
     SHARED
     dlfcn/dlfcn.cpp
)
add_subdirectory(sample)


