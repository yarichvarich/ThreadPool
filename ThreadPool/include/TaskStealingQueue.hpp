#pragma once

#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>

template<typename T> class TaskStealingQueue
{

    using queue = std::deque<T>;

    queue m_queue;
    
    std::mutex m_mutex;

    std::condition_variable m_cv;

public:

    TaskStealingQueue() = default;
    virtual ~TaskStealingQueue() = default;

    TaskStealingQueue& pushFront(T&& in_val);

    bool tryPushFront(T&& in_val);

    bool tryPopFront(T& out_val);

    bool tryPopBack(T& out_val);

    void popFront(T& out_val);

    void popBack(T& out_val);

    size_t size();

    bool empty();
};

#include "TaskStealingQueue.inl"