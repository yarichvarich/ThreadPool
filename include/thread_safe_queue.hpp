#pragma once

#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>

template<typename T> class thread_safe_queue
{

    using queue = std::queue<T>;

    queue m_queue;
    
    std::mutex m_mutex;

    std::condition_variable m_cv;

public:

    thread_safe_queue& push_front(const T& in_val);

    bool try_pop_front();

    void pop_front();

    size_t size();

    bool empty();

    T& front(std::unique_lock<std::mutex>& out_lock);

    T& back(std::unique_lock<std::mutex>& out_lock);
};

#include "..\src\thread_safe_queue.inl"