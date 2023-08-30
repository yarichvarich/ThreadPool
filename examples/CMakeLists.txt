cmake_minimum_required(VERSION 3.21.2)

project(examples)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    $<$<CXX_COMPILER_ID:Clang>:/fsanitize=address,undefined,thread>
    $<$<CXX_COMPILER_ID:GNU>:-fsanitize=undefined,thread>
    $<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO,/fsanitize=address,undefined,thread,/Zi>
    $<$<CXX_COMPILER_ID:Intel>:/fsanitize=address,undefined,thread>
)

add_link_options(
    $<$<CXX_COMPILER_ID:Clang>:/fsanitize=address,undefined,thread>
    $<$<CXX_COMPILER_ID:GNU>:-fsanitize=undefined,thread>
    $<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO,/fsanitize=address,undefined,thread,/Zi>
    $<$<CXX_COMPILER_ID:Intel>:/fsanitize=address,undefined,thread>
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(
    ThreadPool_DIR
    ../ThreadPool/include
)

find_package(ThreadPool CONFIG REQUIRED)

find_package(SFML 2.5 COMPONENTS audio graphics window system REQUIRED)

set(CMAKE_BUILD_TYPE=Release)

set(
    EXAMPLE1_SRC_FILES
    example1.cpp
)

set(
    EXAMPLE2_SRC_FILES
    example2.cpp
)

add_executable(example1 ${EXAMPLE1_SRC_FILES})
target_link_libraries(example1 ThreadPool::ThreadPool)

add_executable(example2 ${EXAMPLE2_SRC_FILES})
target_link_libraries(example2 ThreadPool::ThreadPool sfml-audio sfml-graphics sfml-window sfml-system)