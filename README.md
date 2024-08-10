# OSCompatible library
 C++ operation system compatible library, currently supports thread with priority, policy and CPU cores bind(affinity) on windows and linux

same syntax as std::thread

```cpp
OSCompatible::thread t1( function1 , 5);
t1.join();


OSCompatible::thread::Properties prop = {
    OSCompatible::thread::DEFAULT_PRIORITY,
    OSCompatible::thread::DEFAULT_POLICY,
    OSCompatible::thread::DEFAULT_AFFINITY, // CPU cores the thread should run on
};


OSCompatible::thread t1( prop, function2 , 213);
t1.join();



```