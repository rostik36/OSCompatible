#include <iostream>
#include <thread>
#include <memory>
#include "OSCompatibleThread.h"
#include <time.h>


size_t Calc(size_t a, size_t b)
{
    size_t result = 0;
    if(a < 100)
    {
        result += a + b;
    }
    else
    {
        result += b;
    }
    
    return result;
}




void* Function1(void* arg)
{
    size_t cycles = 0;
    size_t result = 1;
    std::cout << "Function" << std::endl;

    while(cycles < 1000)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        //std::cout << "thread no. " << std::to_string((size_t)arg) << " cycle: " <<
        // std::to_string(cycles) << std::endl;
        result = Calc(result, (size_t)arg);
        ++cycles;
    }
    std::cout << "thread no. " << std::to_string((size_t)arg) << " val: " << 
    std::to_string(result) << std::endl;

    return NULL;
}


bool IsPrime(int n)
{
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; ++i)
    {
        if (n % i == 0) return false;
    }
    return true;
}

void* Function2(void* arg)
{
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate work by performing a heavy computation
    for (int i = 2; i < 10000000; ++i)
    {
        if (IsPrime(i))
        {
            ++count;
        }

        // each 50 iterations, sleep for a short duration to give scheduler 
        // option to context switch to other threads
        if(i%50 == 0)
        {
            // must use duration in 'microseconds' else it will not work as expected
            // it will give other threads a chance to run and finish before the sleep
            // will end.
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Thread no. " << std::to_string((size_t)arg) << 
    " finished with count: " << count << " in " << elapsed.count() << " seconds." << std::endl;

    return NULL;
}





const size_t THREADS_NUM = 20;
const size_t SLOW_THREADS_NUM = 18;



int main(void)
{
    size_t i = 1;
    int policy = SCHED_FIFO;// SCHED_FIFO, SCHED_RR;
    int priority = (sched_get_priority_max(policy) + sched_get_priority_min(policy)) / 2;
    std::shared_ptr<OSCompatibleThread> threads[THREADS_NUM];


    // Allocate threads
    for(i = 0; i < THREADS_NUM; i++)
    {
        threads[i] = std::make_shared<OSCompatibleThread>();
    }

    // Init and run slow threads
    for(i = 1; i <= SLOW_THREADS_NUM; i++)
    {
        if( threads[i]->Init(Function2, (void*)i, sched_get_priority_max(policy), policy, std::vector<bool>{1,1,1,1}) )
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }

    // Init and run the rest of the threads the with the fast configuration
    // (priority and more cores selected)
    for( ; i <= THREADS_NUM; i++)
    {
        if( threads[i]->Init(Function2, (void*)i, sched_get_priority_max(policy), policy, std::vector<bool>{1,1,1,1}) )
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }


    for(i = 1; i <= THREADS_NUM; i++)
    {
        if( threads[i]->Join(nullptr) )
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }

    return 0;
}