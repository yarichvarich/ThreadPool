#pragma once

template<typename F> ThreadPool::AsyncResult<F> ThreadPool::wrapTask(F&& func, ThreadPool::FunctionWrapper::Ptr& outWrappedTask)
{
    typedef typename std::result_of<F()>::type FunctionType;

    std::packaged_task<FunctionType()> task{std::move(func)};

    std::future<FunctionType> result{task.get_future()};

    FunctionWrapper::Ptr wrappedTask{new FunctionWrapper{std::move(task)}};

    outWrappedTask = std::move(wrappedTask);

    return result;
}

template<typename F> ThreadPool::AsyncResult<F> ThreadPool::executeAsync(F&& func)
{
    uint32_t workerID = m_workerID.load();

    workerID = (workerID + 1) % m_workers.size();

    for(uint32_t i = 0; i < m_workers.size(); ++i)
    {
        uint32_t id = (workerID + i) % m_workers.size();
        
        bool result = false;

        auto fut = m_workers[id]->tryAddTask(std::move(func), result);

        if(result)
        {
            return fut;
        }
    }

    return m_workers[workerID]->addTask(std::move(func));
}

template<typename F> ThreadPool::AsyncResultAndFuncWrapper<F> ThreadPool::chainTask(F&& func, FunctionWrapper::Ptr& inoutPreviousTask)
{
    FunctionWrapper::Ptr wrappedTask;
                
    auto result = ThreadPool::wrapTask(func, wrappedTask);

    inoutPreviousTask->then() = std::move(wrappedTask);

    return {std::move(result), &inoutPreviousTask->then()};
}

void ThreadPool::executeAsync(FunctionWrapper::Ptr&& wrappedTask)
{
    uint32_t workerID = m_workerID.load();

    m_workerID = (workerID + 1) % m_workers.size();

    uint32_t K = 2;

    uint32_t i = 0u;

    for(; i < m_workers.size() * K; ++i)
    {
        uint32_t id = (workerID + i) % m_workers.size();
        
        bool result = false;

        result = m_workers[id]->tryAddTask(std::move(wrappedTask));

        if(result)
        {
            return;
        }
    }

    m_workers[workerID]->addTask(std::move(wrappedTask));
}

void ThreadPool::wait()
{
    m_paused = true;
}

void ThreadPool::resume()
{
    while(workersBusy())
    {
        std::this_thread::yield();
    }

    m_paused = false;

    m_cv.notify_all();
}

ThreadPool::ThreadPool(uint32_t numThreads) : m_done{false}, m_paused{true}, m_workerID{0u}
{
    DEBUG_ASSERT(numThreads <= std::thread::hardware_concurrency());

    for(uint32_t workerIndex = 0; workerIndex < numThreads; ++workerIndex)
    {
        m_workers.push_back(std::unique_ptr<Worker>{new Worker{workerIndex, this, &Worker::run}});
    }
}

template<typename F> ThreadPool::AsyncResult<F> ThreadPool::addTasksWithBarrier(std::vector<FunctionWrapper::Ptr>&& tasks, F&& onComplete)
{
    FunctionWrapper::Ptr wrappedTask;
                
    auto result = ThreadPool::wrapTask(onComplete, wrappedTask);

    Barrier* barrier = new Barrier{static_cast<uint32_t>(tasks.size()), std::move(wrappedTask)};

    for(auto& task0 : tasks)
    {
        task0->barrier() = barrier;

        executeAsync(std::move(task0)); 
    }

    return result;
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
    for(auto& worker : m_workers)
    {
        worker->m_done = true;
    }

    while(workersBusy())
    {
        std::this_thread::yield();
    }
}

bool ThreadPool::workersBusy()
{
    for(auto& pWorker : m_workers)
    {
        if(pWorker->busy())
        {
            return true;
        }
    }

    return false;
}