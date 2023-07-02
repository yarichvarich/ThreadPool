cmake_minimum_required(VERSION 3.21.2)

set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

project(thread_pool)

set(
    SRC_FILES

    # src

    src/debug.cpp
    src/thread_safe_queue.inl
    src/thread_pool.cpp

    # include

    include/debug.hpp
    include/thread_safe_queue.hpp
    include/thread_pool.hpp
)

add_library(${PROJECT_NAME} ${SRC_FILES})