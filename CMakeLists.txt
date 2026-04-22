cmake_minimum_required(VERSION 3.10)
project(bookstore_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pipe")

add_executable(code src/code.cpp)

# Ensure executable name is exactly 'code'
set_target_properties(code PROPERTIES OUTPUT_NAME "code")
