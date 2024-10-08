#include <iostream>
#include <thread>
#include <chrono>

#include "OSCompatible.h"



// Example function
int exampleFunction(int a, double b)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(900));

    std::cout << "Function called with arguments: " << a << ", " << b << std::endl;

    return a + static_cast<int>(b);
}


void voidFunction(int a)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(900));

    std::cout << "Void function called with argument: " << a << std::endl;
}

bool boolFunction(int a)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(900));

    std::cout << "Void function called with argument: " << a << std::endl;
    return true;
}




int main()
{
    // Create and start the thread for a function with a return value
    OSCompatible::thread thread1(exampleFunction, 42, 3.14);
    std::cout<< "Thread 1 started" << std::endl;
    thread1.join();

    // Retrieve the result
    std::any result = thread1.getResult();
    if (result.has_value())
    {
        std::cout << "Result: " << std::any_cast<int>(result) << std::endl;
    }

    // Create and start the thread for a void function
    OSCompatible::thread thread2(voidFunction, 42);
    std::cout<< "Thread 2 started" << std::endl;
    thread2.join();

    // No result to retrieve for void function
    std::any voidResult = thread2.getResult();
    if (!voidResult.has_value())
    {
        std::cout << "Void function completed successfully" << std::endl;
    }

    // Create and start the thread for a void function
    OSCompatible::thread thread3(boolFunction, 4277);

    thread3.join();

    // No result to retrieve for void function
    std::any boolResult = thread3.getResult();
    if (boolResult.has_value())
    {
        std::cout << "Result: " << std::any_cast<bool>(boolResult) << std::endl;
    }


    // Create thread with custom properties (priority, policy, affinity)
    OSCompatible::thread::Properties properties = {
        OSCompatible::thread::DEFAULT_PRIORITY,
        OSCompatible::thread::DEFAULT_POLICY,
        OSCompatible::thread::DEFAULT_AFFINITY
    };

    // Create and start the thread for a void function
    OSCompatible::thread thread4(properties, boolFunction, 4277);

    thread4.join();

    // No result to retrieve for void function
    std::any boolResult2 = thread4.getResult();
    if (boolResult2.has_value())
    {
        std::cout << "Result: " << std::any_cast<bool>(boolResult2) << std::endl;
    }

    return 0;
}



