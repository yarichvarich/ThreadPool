#pragma once

#include "../include/thread_safe_queue.hpp"

template<typename T> thread_safe_queue<T>& thread_safe_queue<T>::push_front(const T& in_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    m_queue.push(in_val);

    m_cv.notify_one();

    return *this;
}

template<typename T> bool thread_safe_queue<T>::try_pop_front()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    if(m_queue.empty()) 
    {
        return false;
    }

    m_queue.pop();

    return true;
}

template<typename T> void thread_safe_queue<T>::pop_front()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    m_cv.wait(lk, [this]() { return !m_queue.empty(); });

    m_queue.pop();
}

template<typename T> size_t thread_safe_queue<T>::size()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    return m_queue.size();
}

template<typename T> bool thread_safe_queue<T>::empty()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    return m_queue.empty();
}

template<typename T> T& thread_safe_queue<T>::front(std::unique_lock<std::mutex>& out_lock)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    out_lock = std::move(lk);

    return m_queue.front();    
}

template<typename T> T& thread_safe_queue<T>::back(std::unique_lock<std::mutex>& out_lock)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    out_lock = std::move(lk);

    return m_queue.back();     
}
