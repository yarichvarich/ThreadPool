#include <iostream>
#include <chrono>

#include <ThreadPool.hpp>

int main()
{
    using namespace std::chrono_literals;

    {
        ThreadPool pool{ 5u };
        // setting pool into wait state
        pool.wait();

        std::this_thread::sleep_for(10000ms);

        // pushing lambda into pool`s task queue
        auto fut1 = pool.executeAsync(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#1: " << i << std::endl;
                }

                return 10u;
            }
        );

        // pushing lambda into pool`s task queue
        auto fut2 = pool.executeAsync(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#2: " << i << std::endl;
                }

                return 20u;
            }
        );
        
        // setting pool into running state
        pool.resume();
        /*
        // wait until task1 and task2 are done
        
        std::cout << fut2.get() << " " << fut1.get();
        
        pool.wait();

        std::this_thread::sleep_for(5000ms);

        pool.resume();

        // creating first node in taksChain
        ThreadPool::FunctionWrapper::Ptr taskChain{
            new ThreadPool::FunctionWrapper{
                []() -> uint32_t
                {
                    for (uint32_t i = 0u; i < 100u; ++i)
                    {
                        std::cout << "Executing asynchroniously#3: " << i << std::endl;
                    }

                    return 10u;
                }
            }
        };

        // adding node to task chain
        auto pairedResult = pool.chainTask(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#4: " << i << std::endl;
                }

                return 10u;
            },
            taskChain
        );

        pairedResult = pool.chainTask(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#5: " << i << std::endl;
                }

                return 10u;
            },
            *pairedResult.second
        );

        // pushing taskChain into task queue
        pool.executeAsync(std::move(taskChain));

        auto fut5 = pool.executeAsync(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#6: " << i << std::endl;
                }

                return 10u;
            }
        );
        
        std::this_thread::sleep_for(1000ms); 

        // creating list of tasks
        std::vector<ThreadPool::FunctionWrapper::Ptr> tasks;

        tasks.push_back(
            ThreadPool::FunctionWrapper::Ptr{new ThreadPool::FunctionWrapper(
                    []() -> void
                    {
                        std::cout << "1st barrier function\n";
                    }
                )
            }
        );
        
        tasks.push_back(
            ThreadPool::FunctionWrapper::Ptr{new ThreadPool::FunctionWrapper(
                    []() -> void
                    {
                        std::cout << "2nd barrier function\n";
                    }
                )
            }
        );
        
        tasks.push_back(
            ThreadPool::FunctionWrapper::Ptr{new ThreadPool::FunctionWrapper(
                    []() -> void
                    {
                        std::cout << "3rd barrier function\n";
                    }
                )
            }
        );

        // onComplete will be executed right after execution of task list
        pool.addTasksWithBarrier(std::move(tasks), 
            []() -> void { std::cout << "onComplete!\n"; }
        );

        std::this_thread::sleep_for(1000ms);*/
    }
    std::cout << "Done execution!\n";

    return 0;
}