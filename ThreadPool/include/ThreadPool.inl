#pragma once

template<typename F> ThreadPool::AsyncResult<F> ThreadPool::executeAsync(F&& func)
{
    uint32_t workerID = m_workerID.load();

    workerID = (workerID + 1) % m_workers.size();

    return m_workers[workerID]->addTask(std::move(func));
}

template<typename F> ThreadPool::AsyncResultAndFuncWrapper<F> ThreadPool::chainTask(F&& func, FunctionWrapper::Ptr& inoutPreviousTask)
{
    typedef typename std::result_of<F()>::type FunctionType;

    std::packaged_task<FunctionType()> task{std::move(func)};

    std::future<FunctionType> result{task.get_future()};

    FunctionWrapper::Ptr wrappedTask{new FunctionWrapper{std::move(task)}};

    inoutPreviousTask->then() = std::move(wrappedTask);

    ThreadPool::AsyncResultAndFuncWrapper<F> res;

    return {std::move(result), &inoutPreviousTask->then()};
}

void ThreadPool::executeAsync(FunctionWrapper::Ptr&& wrappedTask)
{
    uint32_t workerID = m_workerID.load();

    workerID = (workerID + 1) % m_workers.size();

    m_workers[workerID]->addTask(std::move(wrappedTask));
}

void ThreadPool::wait()
{
    m_paused = true;
}

void ThreadPool::resume()
{
    m_paused = false;

    m_cv.notify_all();
}

ThreadPool::ThreadPool(uint32_t numThreads) : m_done{false}, m_paused{false}, m_workerID{0u}
{
    DEBUG_ASSERT(numThreads <= std::thread::hardware_concurrency());

    for(uint32_t workerIndex = 0; workerIndex < numThreads; ++workerIndex)
    {
        m_workers.push_back(std::unique_ptr<Worker>{new Worker{workerIndex, this, &Worker::run}});
    }
}

void ThreadPool::addTasksWithBarrier(std::vector<FunctionWrapper::Ptr>&& tasks, FunctionWrapper::Ptr&& onComplete)
{
    Barrier* barrier = new Barrier{static_cast<uint32_t>(tasks.size()), std::move(onComplete)};

    for(auto& task : tasks)
    {
        task->barrier() = barrier;

        executeAsync(std::move(task)); 
    }
}

ThreadPool::~ThreadPool()
{
    m_done = true;
}