project(thread_pool)

set(
    SRC_FILES

    # include

    include/debug.hpp
    include/TaskStealingQueue.hpp
    include/ThreadPool.hpp
)

add_library(${PROJECT_NAME} ${SRC_FILES})