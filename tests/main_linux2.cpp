#include <iostream>
#include <thread>
#include <memory>
#include <iomanip>
#include <sstream>
#include <string>
#include "OSCompatibleThread.h"

std::string NumberToString(size_t number, size_t digitsNum)
{
    std::stringstream ss;
    ss << std::setw(digitsNum)  << number; // optional add << std::setfill('0') before number
    return ss.str();
}


void* Function1(void* arg)
{
    std::cout << "Function1 with arguments: no" << std::endl;
    return NULL;
}


void Function2(int a, double b, int c)
{
    std::cout << "Function with arguments: a=" << a << ", b=" << b << ", c=" << c << std::endl;
}

bool FunctionReturnBool(int a, double b, bool c)
{
    std::cout << "Function3 with arguments: a=" << a << ", b=" << b << ", c=" << c << std::endl;
    return true;
}

int FunctionReturnInt(std::shared_ptr<std::vector<int>> a)
{
    std::cout << "Function4 with argument: ";
    for(size_t j = 0; j < a->size(); j++)
    {
        std::cout << a->at(j) << " ";
    }
    std::cout << std::endl;
    return 23;
}


const size_t THREADS_NUM = 4;




int main(void)
{
    size_t i = 1;
    int policy = SCHED_FIFO; // SCHED_FIFO, SCHED_RR;
    int max_policy = sched_get_priority_max(policy);
    std::shared_ptr<OSCompatibleThread> threads[THREADS_NUM];

    bool res3 = false;
    int res4 = 0;

    // Allocate threads
    for(i = 0; i < THREADS_NUM; i++)
    {
        threads[i] = std::make_shared<OSCompatibleThread>();
    }


    if( threads[0]->Init(max_policy, policy, {1,0,0,0}, Function1, (void*)(i+1)) ) 
    {
        std::cout << threads[0]->GetErrMsg() << std::endl;
    }
    if( threads[0]->Join() )
    {
        std::cout << threads[0]->GetErrMsg() << std::endl;
    }


    if( threads[1]->Init(max_policy, policy, {1,0,0,0}, Function2, 55, 1.52, 13) ) 
    {
        std::cout << threads[1]->GetErrMsg() << std::endl;
    }
    if( threads[1]->Join() )
    {
        std::cout << threads[1]->GetErrMsg() << std::endl;
    }


    if( threads[2]->Init(max_policy, policy, {1,0,0,0}, FunctionReturnBool, -12, 3.14, true) ) 
    {
        std::cout << threads[2]->GetErrMsg() << std::endl;
    }
    if( threads[2]->Join() )
    {
        std::cout << threads[2]->GetErrMsg() << std::endl;
    }
    std::cout << "FunctionReturnBool returned: " << res3 << std::endl;



    if( threads[3]->Init(max_policy, policy, {1,0,0,0}, FunctionReturnInt, std::make_shared<std::vector<int>>(std::initializer_list<int>{5,4,3,2,1})) ) 
    {
        std::cout << threads[3]->GetErrMsg() << std::endl;
    }
    if( threads[3]->Join() )
    {
        std::cout << threads[3]->GetErrMsg() << std::endl;
    }
    std::cout << "FunctionReturnInt returned: " << res4 << std::endl;






    return 0;
}