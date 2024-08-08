#include <iostream>
#include <thread>
#include <memory>
#include <iomanip>
#include <sstream>
#include <string>
#include "../OSCompatible.h"


bool IsPrime(int n)
{
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; ++i)
    {
        if (n % i == 0) return false;
    }
    return true;
}

std::string NumberToString(size_t number, size_t digitsNum)
{
    std::stringstream ss;
    ss << std::setw(digitsNum)  << number; // optional add << std::setfill('0') before number
    return ss.str();
}

/**
 * @brief This function is a worker function for each thread. It performs a heavy computation 
 * by checking if numbers from 2 to 10,000,000 are prime. It also includes a sleep mechanism to 
 * give the scheduler an opportunity to context switch to other threads.
 *
 * @param arg A void pointer to the argument passed to the thread. In this case, it's the thread number.
 *
 * @return A void pointer. In this case, it always returns NULL.
 */
void* Function(void* arg)
{
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate work by performing a heavy computation
    for (int i = 2; i < 100000; ++i)
    {
        if (IsPrime(i))
        {
            ++count;
        }

        // each 1000 iterations, sleep for a short duration to give scheduler 
        // option to context switch to other threads
        if(i%1000 == 0)
        {
            // must use duration in 'microseconds' else it will not work as expected
            // it will give other threads a chance to run and finish before the sleep
            // will end.
            std::this_thread::sleep_for(std::chrono::microseconds(5));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Thread no. " << NumberToString((size_t)arg, 2) << 
    " finished with count: " << count << " in " << elapsed.count() << " seconds." << std::endl;

    return NULL;
}


void Function1(int a, double b, int c)
{
    std::cout << "Function with arguments: a=" << a << ", b=" << b << ", c=" << c << std::endl;
}


const size_t SLOW_THREADS_NUM = 2;
const size_t THREADS_NUM = 20;




int main(void)
{
    size_t i = 1;
    int policy = SCHED_FIFO; // SCHED_FIFO, SCHED_RR;
    int min_policy = sched_get_priority_min(policy);
    int max_policy = sched_get_priority_max(policy);
    int priority = (max_policy + min_policy) / 2;
    std::shared_ptr<OSCompatibleThread> threads[THREADS_NUM];


    // Allocate threads
    for(i = 0; i < THREADS_NUM; i++)
    {
        threads[i] = std::make_shared<OSCompatibleThread>();
    }

    // Init and run slow threads
    for(i = 0; i < SLOW_THREADS_NUM; i++)
    {
        if( threads[i]->Init(max_policy, policy, {1,0,0,0}, Function, (void*)(i+1)) ) 
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }

    // Init and run the rest of the threads the with the fast configuration
    // (priority and more cores selected)
    for( ; i < THREADS_NUM; i++)
    {
        if( threads[i]->Init(max_policy, policy, {1,1,0,0}, Function, (void*)(i+1)) )
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }


    // Join threads
    for(i = 0; i < THREADS_NUM; i++)
    {
        if( threads[i]->Join(nullptr) )
        {
            std::cout << threads[i]->GetErrMsg() << std::endl;
        }
    }

    return 0;
}