#pragma once

template<typename T> TaskStealingQueue<T>& TaskStealingQueue<T>::pushFront(T&& in_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    m_queue.push_front(std::forward<T>(in_val));

    m_cv.notify_one();

    return *this;
}

template<typename T> bool TaskStealingQueue<T>::tryPopFront(T& out_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    if(m_queue.empty()) 
    {
        return false;
    }

    out_val = std::move(m_queue.front());

    m_queue.pop_front();

    return true;
}

template<typename T> bool TaskStealingQueue<T>::tryPopBack(T& out_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    if(m_queue.size() <= 1u)
    {
        return false;
    }

    out_val = std::move(m_queue.back());

    m_queue.pop_back();

    return true;
}

template<typename T> void TaskStealingQueue<T>::popFront(T& out_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    m_cv.wait(lk, [this]() { return !m_queue.empty(); });

    out_val = std::move(m_queue.front());

    m_queue.pop_front();
}

template<typename T> void TaskStealingQueue<T>::popBack(T& out_val)
{
    std::unique_lock<std::mutex> lk(m_mutex);

    m_cv.wait(lk, [this]() { return m_queue.size() > 1u; });

    out_val = std::move(m_queue.back());

    m_queue.pop_back();
}

template<typename T> size_t TaskStealingQueue<T>::size()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    return m_queue.size();
}

template<typename T> bool TaskStealingQueue<T>::empty()
{
    std::unique_lock<std::mutex> lk(m_mutex);

    return m_queue.empty();
}
