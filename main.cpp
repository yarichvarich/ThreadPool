#include <iostream>
#include <chrono>

#include "ThreadPool/include/ThreadPool.hpp"

int main()
{
    using namespace std::chrono_literals;

    {
        ThreadPool pool{ 2u };

        /*auto fut1 = pool.executeAsync(
            []() -> uint32_t
            {
                for (uint32_t i = 0u; i < 100u; ++i)
                {
                    std::cout << "Executing asynchroniously#1: " << i << std::endl;
                }

                return 10u;
            }
        );

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

        std::cout << fut2.get() << " " << fut1.get();
        
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
        
        std::this_thread::sleep_for(1000ms); */

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

        pool.addTasksWithBarrier(std::move(tasks), ThreadPool::FunctionWrapper::Ptr{ new ThreadPool::FunctionWrapper{ []() -> void { std::cout << "onComplete!\n"; } } });

        std::this_thread::sleep_for(1000ms);
    }
    std::cout << "Done execution!\n";

    return 0;
}