set(CMAKE_REQUIRED_FLAGS -std=c++17)
set(CMAKE_REQUIRED_LIBRARIES -pthread) 

add_library(ThreadPool::ThreadPool INTERFACE IMPORTED)

target_include_directories(ThreadPool::ThreadPool INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_compile_features(ThreadPool::ThreadPool INTERFACE cxx_std_17)

