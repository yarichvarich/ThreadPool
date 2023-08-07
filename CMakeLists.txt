project(tests)

cmake_minimum_required(VERSION 3.21.2)

set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(
    SRC_FILES

    main.cpp
)

set(
    THREAD_POOL_LIB

    ThreadPool
)

set(
    ThreadPool_DIR

    ./ThreadPool/include
)

find_package(ThreadPool CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ThreadPool::ThreadPool)

install(TARGETS ${PROJECT_NAME} DESTINATION ./bin)