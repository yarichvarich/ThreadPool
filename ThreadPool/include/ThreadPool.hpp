#pragma once

#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>
#include <future>
#include <memory>
#include <unordered_map>

#include "TaskStealingQueue.hpp"
#include "debug.hpp"

class ThreadPool
{

public:

    class Barrier;

    class FunctionWrapper
    {
        public:

            using Ptr = std::unique_ptr<FunctionWrapper>;

        private:

            struct BaseType
            {
                virtual void invoke() = 0;

                virtual ~BaseType() = default;

            };

            template<typename F> struct ImplType : public BaseType
            {
                F m_func;

                ImplType(F&& func) : m_func{std::move(func)} {}

                void invoke() override { m_func(); }
            };

            std::unique_ptr<BaseType> m_impl;

            Ptr m_then;

            Barrier* m_pBarrier = nullptr;

        public:

            template<typename F> FunctionWrapper(F&& f) : m_impl{new ImplType<F>(std::move(f))} {}

            FunctionWrapper() = default;

            FunctionWrapper(FunctionWrapper&& rr, Barrier* pBarrier = nullptr) : m_impl{std::move(rr.m_impl)}, m_then{std::move(rr.m_then)}, m_pBarrier{pBarrier} {}

            FunctionWrapper& operator=(FunctionWrapper&& rr)
            {
                m_impl = std::move(rr.m_impl);

                m_then = std::move(rr.m_then);

                m_pBarrier = rr.m_pBarrier;

                rr.m_pBarrier = nullptr;

                return *this;
            }

            Ptr& then()
            {
                return m_then;
            }

            Barrier*& barrier()
            {
                return m_pBarrier;
            }

            FunctionWrapper(const FunctionWrapper& other) = delete;
            FunctionWrapper& operator=(const FunctionWrapper& other) = delete;

            void operator()() 
            { 
               m_impl->invoke();

               if(m_pBarrier != nullptr)
               {
                    m_pBarrier->increment();
               }
            }
        
        friend class Worker;
    };

    template<typename F> using AsyncResult = std::future<typename std::result_of<F()>::type>;

    template<typename F> using AsyncResultAndFuncWrapper = std::pair<typename AsyncResult<F>, FunctionWrapper::Ptr*>;

    class Worker
    {
        private:

            ThreadPool* m_poolPtr;

            std::unique_ptr<TaskStealingQueue<FunctionWrapper::Ptr>> m_tasks;

            uint32_t m_threadId;

            std::atomic<bool> m_done;

            std::unique_ptr<std::thread> m_thread;

        public:

            Worker() = default;

            template<typename ReturnType, typename... Args> Worker(uint32_t ID, ThreadPool* poolPtr, ReturnType&& func, Args&&... args) : m_poolPtr{poolPtr}, m_tasks{new TaskStealingQueue<FunctionWrapper::Ptr>{}}, m_thread{new std::thread{std::forward<ReturnType>(func), this, std::forward<Args>(args)...}}, m_threadId{ID}, m_done{false}  
            {
            }

            bool trySteal(FunctionWrapper::Ptr& outFunc)
            {
                if(m_tasks->tryPopBack(outFunc))
                {
                    return true;
                }
                return false;
            }

            bool popFromLocalQueue(FunctionWrapper::Ptr& task)
            {
                if(m_tasks->tryPopFront(task))
                {
                    return true;
                }

                return false;
            }

            bool popFromOtherWorker(FunctionWrapper::Ptr& task)
            {
                for(uint32_t i = 0; i < m_poolPtr->m_workers.size(); ++i)
                {
                    uint32_t stealFromID = (m_threadId + 1 + i) % m_poolPtr->m_workers.size();

                    if(m_poolPtr->m_workers[stealFromID]->trySteal(task))
                    {
                        return true;
                    }
                }

                return false;
            }

            void runTask(FunctionWrapper::Ptr& task)
            {
                (*task)();

                if(task->then() != nullptr)
                {
                    m_poolPtr->executeAsync(std::forward<FunctionWrapper::Ptr>(task->then()));
                }
            }

            void run()
            {
                while(!m_poolPtr->m_done)
                {
                    FunctionWrapper::Ptr task{};
                    
                    if(popFromLocalQueue(task))
                    {
                        runTask(task);
                    }
                    else if(popFromOtherWorker(task))
                    {
                        runTask(task);
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            }

            bool done()
            {
                return m_done.load();
            }

            template<typename F> ThreadPool::AsyncResult<F> addTask(F&& func)
            {
                typedef typename std::result_of<F()>::type FunctionType;

                std::packaged_task<FunctionType()> task{std::move(func)};

                std::future<FunctionType> result{task.get_future()};

                FunctionWrapper::Ptr wrappedTask{new FunctionWrapper{std::move(task)}};

                m_tasks->pushFront(std::move(wrappedTask));

                return result;
            }

            template<typename F> ThreadPool::AsyncResult<F> tryAddTask(F&& func, bool& tryResult)
            {
                typedef typename std::result_of<F()>::type FunctionType;

                std::packaged_task<FunctionType()> task{std::move(func)};

                std::future<FunctionType> result{task.get_future()};

                FunctionWrapper::Ptr wrappedTask{new FunctionWrapper{std::move(task)}};

                tryResult = m_tasks->tryPushFront(std::move(wrappedTask));

                return result;
            }

            void addTask(FunctionWrapper::Ptr&& wrappedTask)
            {
                m_tasks->pushFront(std::move(wrappedTask));
            }

            bool tryAddTask(FunctionWrapper::Ptr&& wrappedTask)
            {
                return m_tasks->tryPushFront(std::move(wrappedTask));
            }

            ~Worker()
            {
                m_done = true;

                m_thread->join();
            }
    };

    class Barrier
    {
        private:

            uint32_t m_currentCount = 0u;
            
            uint32_t m_requiredCount;

            FunctionWrapper::Ptr m_onComplete;

            std::mutex m_mut;

        public:

            Barrier() = default;

            Barrier(uint32_t requiredCount, FunctionWrapper::Ptr&& onComplete) : m_requiredCount{requiredCount}, m_onComplete{std::move(onComplete)} {}

            ~Barrier()
            {
                if(m_onComplete != nullptr)
                {
                    (*m_onComplete)();
                }
            }

            void increment()
            {
                bool toDestroy = false;

                {
                    std::lock_guard<std::mutex> lk{m_mut};

                    ++m_currentCount;

                    toDestroy = m_currentCount == m_requiredCount;
                }

                if(toDestroy)
                {
                    delete this;
                }
            }
    };

private:

    std::mutex m_mut;

    std::condition_variable m_cv;

    std::atomic<bool> m_paused;

    std::atomic<bool> m_done;

    std::atomic<uint32_t> m_workerID;

    std::vector<std::unique_ptr<Worker>> m_workers;

public:

    ThreadPool() = default;

    ThreadPool(uint32_t numThreads);

    template<typename F> AsyncResult<F> executeAsync(F&& func);

    void executeAsync(FunctionWrapper::Ptr&& wrappedTask);

    template<typename F> AsyncResultAndFuncWrapper<F> chainTask(F&& func, FunctionWrapper::Ptr& inoutPreviousTask);

    void addTasksWithBarrier(std::vector<FunctionWrapper::Ptr>&& tasks, FunctionWrapper::Ptr&& onComplete);

    void wait();

    void resume();

    ~ThreadPool();

};

#include "ThreadPool.inl"