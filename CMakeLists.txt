cmake_minimum_required(VERSION 3.21.2)

project(tests)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
        #$<$<CXX_COMPILER_ID:Clang>:/fsanitize=address>

        #$<$<CXX_COMPILER_ID:GNU>:-fsanitize=address>

        #$<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO /fsanitize=address /Zi>

        #$<$<CXX_COMPILER_ID:Intel>:/fsanitize=address>
)

add_link_options(
    
        #$<$<CXX_COMPILER_ID:Clang>:>

        $<$<CXX_COMPILER_ID:GNU>:-fsanitize=undefined>

        #$<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO>

        #$<$<CXX_COMPILER_ID:Intel>:>
    
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(yes.)
else()
    message(no.)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(
    SRC_FILES

    main.cpp
)

set(
    ThreadPool_DIR

    ./ThreadPool/include
)

find_package(ThreadPool CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ThreadPool::ThreadPool)