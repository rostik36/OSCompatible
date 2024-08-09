/**
 * @file thread.h
 * 
 * @brief Class to manage OS-compatible threads with priority, policy, and CPU 
 * core assignment.
 * 
 * @author Rostik
 * @version 0.4
 * @date 2024-07-27
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __thread__
#define __thread__

#include <functional>
#include <future>
#include <any>

#ifdef _WIN32       // Windows
#include <windows.h>
#else               // Linux
#include <pthread.h>
#endif


namespace OSCompatible
{


/**
 * @brief Class to manage OS-compatible threads with priority, policy, and CPU 
 * core assignment.
 * 
 * This class provides a platform-independent interface for managing threads, 
 * allowing the user to set thread priority, scheduling policy, and CPU core 
 * assignment.
 * 
 * The class provides methods for initializing and starting a new thread, waiting 
 * for the thread to finish, retrieving the thread's return value, setting thread 
 * priority, scheduling policy, and CPU cores, and retrieving the thread's unique 
 * identifier.
 * 
 * @note Supports both Windows and POSIX operating systems(Linux).
 * 
 * @warning Don't allocate many OSCompatible thread on the stack, stack can run 
 * out and you will receive segmentation fault or some unexpected behavior.
 */
class thread
{
public:
    // Define handle threadId type based on the operating system
    #ifdef _WIN32       // Windows
    typedef DWORD       threadId;
    #else               // Linux
    typedef pthread_t   threadId;
    #endif

    /**
     * @brief Structure to hold thread properties such as priority, policy, and
     * CPU affinity (CPU cores to which the thread pinned and will be running on).
     */
    struct Properties
    {
        int priority;
        int policy;
        std::vector<bool> affinity; // CPU affinity (CPU cores to which the thread pinned and will be running on)
    };

    static const int DEFAULT_PRIORITY;
    static const int DEFAULT_POLICY;
    static const std::vector<bool> DEFAULT_AFFINITY; // No CPU affinity (thread will be running on all available CPU cores)
    static const Properties DEFAULT_PROPERTIES;


    /**
     * @brief Default constructor for the thread class.
     *
     * Initializes the thread object with default values.
     * The m_thread handle is set to pthread_t(), indicating that the thread is 
     * not joinable.
     * The m_func pointer is set to nullptr, indicating that no function is 
     * associated with the thread.
     * A new std::promise and std::future are created to handle the return 
     * value (if any) from the thread function.
     * The std::promise is stored in a shared_ptr, and the std::future is 
     * stored as a member variable.
     *
     * @note The default constructed thread object is not joinable.
     * @note The m_func pointer is set to nullptr, indicating that no function i
     * s associated with the thread.
     * @note The std::promise and std::future are used to handle the return 
     * value (if any) from the thread function.
     */
    thread();


    /**
     * @brief Constructor for the thread class that takes a function
     * and its arguments.
     *
     * This constructor creates a new thread and executes the provided function
     * with the given arguments.
     * The function's return value (if any) is captured and stored in a 
     * std::future for later retrieval.
     *
     * @tparam Function The type of the function to be executed in the new thread.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param func The function to be executed in the new thread.
     * @param args The arguments to be passed to the function.
     *
     * @throws std::runtime_error If the thread cannot be created or if no 
     * function is provided.
     *
     * @note The function and its arguments are stored in a std::function object
     *  and bound to the new thread.
     * The std::function object is then wrapped in a lambda function that 
     * catches any exceptions thrown during
     * the execution of the provided function and sets the corresponding 
     * std::promise with the exception.
     *
     * @note If the function's return type is void, an empty std::any is set as 
     * the promise's value.
     * Otherwise, the function's return value is wrapped in a std::any and set 
     * as the promise's value.
     *
     * @note The new thread is created using the pthread_create function, and 
     * the threadFuncWrapper is
     * used as the thread's entry point. The threadFuncWrapper function calls 
     * the stored std::function
     * object and cleans up any allocated resources.
     *
     * @note The constructor is marked as a template function to support 
     * functions with different argument types.
     */
    template <typename Function, typename... Args, typename = std::enable_if_t<std::is_invocable_v<Function, Args...>> >
    thread(Function&& func, Args&&... args);
    // enable_if_t with std::is_invocable_v ensures that the function is 
    // callable with the provided arguments.
    // so the the deduction process done right, means the compiler will
    // automatically find the correct template constructor for given arguments.


    /**
     * @brief Constructor for the thread class that takes a function,
     *  its arguments, and thread properties.
     *
     * This constructor creates a new thread with the provided properties and 
     * executes the provided function
     * with the given arguments. The function's return value (if any) is 
     * captured and stored in a std::future for later retrieval.
     *
     * @tparam Function The type of the function to be executed in the new thread.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param properties The properties for the new thread, including priority,
     *  policy, and CPU affinity.
     * @param func The function to be executed in the new thread.
     * @param args The arguments to be passed to the function.
     *
     * @throws std::runtime_error If the thread cannot be created or if no 
     * function is provided.
     *
     * @note The function and its arguments are stored in a std::function object
     *  and bound to the new thread.
     * The std::function object is then wrapped in a lambda function that catches
     *  any exceptions thrown during
     * the execution of the provided function and sets the corresponding 
     * std::promise with the exception.
     *
     * @note If the function's return type is void, an empty std::any is set as 
     * the promise's value.
     * Otherwise, the function's return value is wrapped in a std::any and set 
     * as the promise's value.
     *
     * @note The new thread is created using the pthread_create function, and 
     * the threadFuncWrapper is
     * used as the thread's entry point. The threadFuncWrapper function calls 
     * the stored std::function
     * object and cleans up any allocated resources.
     *
     * @note The constructor is marked as a template function to support 
     * functions with different argument types.
     * 
     *    
     * @warning Windows does not support Policy, so policy will be ignored and not effect nothing.
     * 
     * @warning Linux will need privileges to set priority, policy and CPU cores
     * @warning otherwise will return error about Operation not permitted. (use sudo)
     * 
     * @warning Don't allocate many OSCompatibleThread on the stack, stack can run out 
     * @warning and you will receive segmentation fault or some unexpected behavior.
     * 
     */
    template <typename Function, typename... Args>
    thread(const Properties& properties, Function&& func, Args&&... args);


    // Deleting copy constructor and assignment operator
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    /**
     * @brief Move constructor for the thread class.
     * This constructor moves the resources from another thread object 
     * to the newly created object.
     * After the move, the other object is left in a valid but unspecified state.
     *
     * @param other The other thread object from which to move the resources.
     * @throws No exceptions are thrown by this constructor.
     */
    thread(thread&& other) noexcept;


    thread& operator=(thread&& other) noexcept;


    /**
     * @brief Joins the calling thread with the thread represented by the object.
     *
     * This function blocks the calling thread until the thread represented by the
     * object terminates. If the thread has already terminated, the function returns
     * immediately.
     *
     * @throws std::runtime_error If the calling thread cannot join with the
     * represented thread.
     *
     * @note After a successful join, the thread handle (m_thread) is reset to
     * pthread_t(), indicating that the thread is no longer joinable.
     */
    void join();


    /**
     * @brief Detaches the thread from the calling process.
     *
     * This function detaches the calling thread from the process, allowing it to
     * continue execution independently. After a thread is detached, it can no longer
     * be joined.
     *
     * @throws std::runtime_error If the thread cannot be detached.
     *
     * @note Detaching a thread is useful when the thread's execution is independent
     * of the main program's flow, and the program does not need to wait for the thread
     * to complete.
     *
     * @note After detaching a thread, the thread handle (m_thread) is reset to
     * pthread_t(), indicating that the thread is no longer joinable.
     */
    void detach();

    
    /**
     * @brief Checks if the thread is joinable.
     *
     * This function determines if the thread represented by the object is joinable.
     * A thread is considered joinable if it has not been detached and has not yet
     * terminated.
     *
     * @return true if the thread is joinable, false otherwise.
     *
     * @note A joinable thread can be joined using the join() member function.
     * Once a thread has been joined, it is no longer joinable.
     *
     * @note A thread that has been detached is no longer joinable.
     *
     * @note The default constructed thread object is not joinable.
     */
    bool joinable() const;


    /**
     * @brief Retrieves the result from the future associated with the thread.
     *
     * This function blocks the calling thread until the associated future's result
     * becomes available. If the future's result is a value, it is returned as std::any.
     * If the future's result is an exception, the exception is rethrown.
     *
     * @return The result of the future as std::any. If the future's result is an
     * exception, the exception is rethrown.
     *
     * @note This function should be called only after the thread has completed its
     * execution and the future's result is available.
     *
     * @note If the thread is not joinable, calling this function will result in
     * undefined behavior.
     *
     * @note If the thread's function is a void function, this function will return
     * an empty std::any.
     *
     * @note The future's result is consumed by this function, and the future is no
     * longer valid after this call.
     *
     * @throws Any exception that was set as the future's result.
     *
     * @see std::future, std::promise, std::any
     */
    std::any getResult();

private:
    // Wrapper function to be passed to pthread_create
    static void* threadFuncWrapper(void* arg)
    {
        auto* func = static_cast<std::function<void()>*>(arg);
        (*func)();
        delete func;
        return nullptr;
    }

    threadId m_thread; // thread handle
    std::function<void()> m_func;
    // used for the return value (if exists)
    std::shared_ptr<std::promise<std::any>> m_promise;
    std::future<std::any> m_future;
    Properties m_properties; // Additional properties for the thread, if needed
};



const int thread::DEFAULT_PRIORITY = 255;
const int thread::DEFAULT_POLICY = 255;
const std::vector<bool>  thread::DEFAULT_AFFINITY = {}; // No CPU affinity (thread will be running on all available CPU cores)
const thread::Properties thread::DEFAULT_PROPERTIES = {DEFAULT_PRIORITY, DEFAULT_POLICY, DEFAULT_AFFINITY};





thread::thread()
    : 
    m_thread(),
    m_func(nullptr),
    m_promise(std::make_shared<std::promise<std::any>>()),
    m_future(m_promise->get_future()),
    m_properties(DEFAULT_PROPERTIES)
{ }


// template <typename Function, typename... Args>
// thread::thread(Function&& func, Args&&... args)
//     :
//     thread()
// {
//     using ReturnType = std::invoke_result_t<Function, Args...>;

//     auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

//     m_func = [this, boundFunc]() {
//         try
//         {
//             if constexpr (std::is_void_v<ReturnType>)
//             {
//                 boundFunc();
//                 m_promise->set_value(std::any{});
//             }
//             else
//             {
//                 m_promise->set_value(boundFunc());
//             }
//         }
//         catch (...)
//         {
//             m_promise->set_exception(std::current_exception());
//         }
//     };

//     if (m_func)
//     {
//         auto m_funcptr = new std::function<void()>(std::move(m_func));
        
//         if (pthread_create(&m_thread, nullptr, threadFuncWrapper, m_funcptr) != 0)
//         {
//             delete m_funcptr; // Clean up in case of error
//             throw std::runtime_error("Failed to create thread");
//         }
//     }
//     else
//     {
//         throw std::runtime_error("No function to execute in thread");
//     }
// }


// template <typename Function, typename... Args>
// thread::thread(const Properties& properties, Function&& func, Args&&... args)
//     :
//     m_thread(),
//     m_func(nullptr),
//     m_promise(std::make_shared<std::promise<std::any>>()),
//     m_future(m_promise->get_future()),
//     m_properties(properties)
// {
//     using ReturnType = std::invoke_result_t<Function, Args...>;

//     auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

//     m_func = [this, boundFunc]() {
//         try
//         {
//             if constexpr (std::is_void_v<ReturnType>)
//             {
//                 boundFunc();
//                 m_promise->set_value(std::any{});
//             }
//             else
//             {
//                 m_promise->set_value(boundFunc());
//             }
//         }
//         catch (...)
//         {
//             m_promise->set_exception(std::current_exception());
//         }
//     };

//     if (m_func)
//     {
//         auto m_funcptr = new std::function<void()>(std::move(m_func));
//         #ifdef _WIN32   // Windows

//         #else
//         if (pthread_create(&m_thread, nullptr, threadFuncWrapper, m_funcptr) != 0)
//         {
//             delete m_funcptr; // Clean up in case of error
//             throw std::runtime_error("Failed to create thread");
//         }
//         #endif

//     }
//     else
//     {
//         throw std::runtime_error("No function to execute in thread");
//     }
// }

template <typename Function, typename... Args, typename = std::enable_if_t<std::is_invocable_v<Function, Args...>> >
thread::thread(Function&& func, Args&&... args)
    : thread() // Call default constructor first
{
    using ReturnType = std::invoke_result_t<Function, Args...>;

    auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

    m_func = [this, boundFunc]() {
        try {
            if constexpr (std::is_void_v<ReturnType>) {
                boundFunc();
                m_promise->set_value({});
            } else {
                m_promise->set_value(boundFunc());
            }
        } catch (...) {
            m_promise->set_exception(std::current_exception());
        }
    };

    #ifdef _WIN32
    // Windows-specific thread creation
    m_thread = CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(threadFuncWrapper), &m_func, 0, nullptr);
    if (m_thread == nullptr) {
        throw std::runtime_error("Failed to create thread");
    }
    #else
    // POSIX-specific thread creation
    if (pthread_create(&m_thread, nullptr, threadFuncWrapper, &m_func) != 0) {
        throw std::runtime_error("Failed to create thread");
    }
    #endif
}

template <typename Function, typename... Args>
thread::thread(const Properties& properties, Function&& func, Args&&... args)
    : thread(std::forward<Function>(func), std::forward<Args>(args)...)
{
    m_properties = properties;

    #ifdef _WIN32
    
    #else
    // On Linux, set the thread's scheduling policy, priority, and CPU affinity
    struct sched_param sched;
    sched.sched_priority = m_properties.priority;

    if (pthread_setschedparam(m_thread, m_properties.policy, &sched) != 0) {
        throw std::runtime_error("Failed to set thread priority and policy");
    }

    
    #endif
}


void thread::SetPriority(const thread::Properties& properties)
{
#ifdef _WIN32
    
#else   // Linux
    struct sched_param sched;
    sched.sched_priority = m_properties.priority;

    if (pthread_setschedparam(m_thread, m_properties.policy, &sched) != 0)
    {
        throw std::runtime_error("Failed to set thread priority and policy");
    }
#endif
}


void thread::SetPolicy(const thread::Properties& properties)
{
#ifdef _WIN32
    
#else   // Linux
    struct sched_param sched;
    sched.sched_priority = m_properties.priority;

    if (pthread_setschedparam(m_thread, m_properties.policy, &sched) != 0)
    {
        throw std::runtime_error("Failed to set thread priority and policy");
    }
#endif
}

void thread::SetAffinity(const thread::Properties& properties)
{
#ifdef _WIN32
    if (!m_properties.affinity.empty())
    {
        DWORD_PTR mask = 0;

        for (size_t i = 0; i < m_properties.affinity.size(); ++i)
        {
            if (m_properties.affinity[i])
            {
                mask |= (1 << i);
            }
        }

        SetThreadAffinityMask(m_thread, mask);
    }

#else // Linux
    if (!m_properties.affinity.empty())
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        for (size_t i = 0; i < m_properties.affinity.size(); ++i)
        {
            if (m_properties.affinity[i]) {
                CPU_SET(i, &cpuset);
            }
        }

        if (pthread_setaffinity_np(m_thread, sizeof(cpu_set_t), &cpuset) != 0)
        {
            throw std::runtime_error("Failed to set thread affinity");
        }
    }
}





thread::thread(thread&& other) noexcept
    :
    m_thread(other.m_thread),
    m_func(std::move(other.m_func)),
    m_promise(std::move(other.m_promise)),
    m_future(std::move(other.m_future))
{
    other.m_thread = pthread_t();
}


thread& thread::operator=(thread&& other) noexcept
{
    if (this != &other)
    {
        if (joinable())
        {
            join();
        }

        m_thread = other.m_thread;
        m_func = std::move(other.m_func);
        m_promise = std::move(other.m_promise);
        m_future = std::move(other.m_future);
        other.m_thread = pthread_t(); // Reset other's thread handle
    }
    return *this;
}


void thread::join()
{
    if (pthread_join(m_thread, nullptr) != 0)
    {
        throw std::runtime_error("Failed to join thread");
    }
    m_thread = pthread_t(); // Reset the thread handle
}


void thread::detach()
{
#ifdef _WIN32   // Windows

    if (CloseHandle(reinterpret_cast<HANDLE>(m_thread)) == FALSE)
    {
        throw std::runtime_error("Failed to close thread handle");
    }
    
    m_thread = nullptr; // Reset the thread handle


#else           // Linux

    if (pthread_detach(m_thread) != 0)
    {
        throw std::runtime_error("Failed to detach thread");
    }
    
    m_thread = pthread_t(); // Reset the thread handle
#endif
}


bool thread::joinable() const
{
    return m_thread != pthread_t();
}


std::any thread::getResult()
{
    return m_future.get();
}


}


#endif //__thread__