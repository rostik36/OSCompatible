/**
 * @file thread.h
 * 
 * @brief Class to manage OS-compatible threads with priority, policy, and CPU 
 * core assignment.
 * @note Supported since C++17, because used futures like std::any, std::invoke_result_t and std:Lis_invocable_v
 * 
 * @author Rostik
 * @version 1.3
 * @date 2024-07-27
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __thread__
#define __thread__
#include <type_traits> // is_invocable_v
#include <functional>
#include <future>
#include <any>
#include <cstring>

#include <stdexcept>

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
 * @note Supported since C++17, because used futures like std::any, std::invoke_result_t and std:Lis_invocable_v
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
    template <typename Function, typename... Args, std::enable_if_t<std::is_invocable_v<Function, Args...>, int> = 0 >
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
    void SetPriority(const Properties& properties);
    void SetPolicy(const Properties& properties);
    void SetAffinity(const Properties& properties);


    // Wrapper function to be passed to pthread_create
    static void* threadFuncWrapper(void* arg)
    {
        auto* func = static_cast<std::function<void()>*>(arg);
        (*func)();
        delete func;
        return nullptr;
    }

#ifdef _WIN32
    HANDLE m_handle;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_releaseThread;
    bool m_propertiesInitialized = true;
#else // Unix (Linux)
    pthread_t m_handle;
    pthread_attr_t m_attr;
#endif
    bool m_initialized;
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
#ifdef _WIN32
    m_handle(nullptr),
#else   // Unix (Linux)
    m_handle(),
#endif
    m_func(nullptr),
    m_promise(std::make_shared<std::promise<std::any>>()),
    m_future(m_promise->get_future()),
    m_properties(DEFAULT_PROPERTIES)
{ }



template <typename Function, typename... Args, std::enable_if_t<std::is_invocable_v<Function, Args...>, int> >
thread::thread(Function&& func, Args&&... args)
    :
#ifdef _WIN32
    m_handle(nullptr),
    m_mutex(),
    m_cv(),
    m_releaseThread(false),
    m_propertiesInitialized(true),
#else   // Unix (Linux)
    m_handle(),
#endif
    m_initialized(false),
    m_func(nullptr),
    m_promise(std::make_shared<std::promise<std::any>>()),
    m_future(m_promise->get_future()),
    m_properties(DEFAULT_PROPERTIES)
{
    using ReturnType = std::invoke_result_t<Function, Args...>;

    auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

    m_func = [this, boundFunc]()
    {
        try
        {
            if constexpr (std::is_void_v<ReturnType>)
            {
                boundFunc();
                m_promise->set_value({});
            }
            else
            {
                m_promise->set_value(boundFunc());
            }
        }
        catch (...)
        {
            m_promise->set_exception(std::current_exception());
        }
    };

    auto m_funcptr = new std::function<void()>(std::move(m_func));
    
#ifdef _WIN32
    // Windows-specific thread creation
    m_handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(threadFuncWrapper), m_funcptr, 0, nullptr);
    if (m_handle == nullptr)
    {
        throw std::runtime_error("Failed to create thread");
    }
#else
    // POSIX-specific thread creation
    if (pthread_create(&m_handle, nullptr, threadFuncWrapper, m_funcptr) != 0)
    {
        throw std::runtime_error(std::string("Failed to create thread :") + std::string(strerror(errno)));
    }
#endif

    m_initialized = true;
}




// To try to create the thread with properties must create wrapper function that blocked with barrier
// to try to set thread properties and if all setted correctly it will release the barrier
// and pass if statement and call the actual function passed

template <typename Function, typename... Args>
thread::thread(const Properties& properties, Function&& func, Args&&... args)
    :
#ifdef _WIN32
    m_handle(nullptr),
    m_mutex(),
    m_cv(),
    m_releaseThread(false),
    m_propertiesInitialized(false),
#else   // Unix (Linux)
    m_handle(),
#endif
    m_initialized(false),
    m_func(nullptr),
    m_promise(std::make_shared<std::promise<std::any>>()),
    m_future(m_promise->get_future()),
    m_properties(properties)
{
    using ReturnType = std::invoke_result_t<Function, Args...>;

    auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);


#ifdef _WIN32
    m_func = [this, boundFunc]()
    {
        // must set here barrier to wait until all the properties are initialized
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_releaseThread; });

        if(m_propertiesInitialized)
        {
            try
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    boundFunc();
                    m_promise->set_value({});
                }
                else
                {
                    m_promise->set_value(boundFunc());
                }
            }
            catch (...)
            {
                m_promise->set_exception(std::current_exception());
            }
        }
    };

    auto m_funcptr = new std::function<void()>(std::move(m_func));

    // Windows-specific thread creation
    m_handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(threadFuncWrapper), m_funcptr, 0, nullptr);
    if (m_handle == nullptr)
    {
        throw std::runtime_error("Failed to create thread");
    }

    try
    {
        // Try to set thread properties
        //SetName(properties);
        SetPolicy(properties);
        SetPriority(properties);
        SetAffinity(properties);

        m_releaseThread = true;
        m_propertiesInitialized = true;
        m_cv.notify_one(); // Release the waiting thread after setting properties
    }
    catch(std::exception& e)
    {
        m_releaseThread = true;
        m_propertiesInitialized = false; // Properties not setted correctly
        m_cv.notify_one(); // Release the waiting thread after failed setting properties

        join(); // Wait for the thread to finish

        // If properties are not set correctly, throw an exception
        throw std::runtime_error("Failed to set thread properties: " + std::string(e.what()));
    }

    m_initialized = true;


#else   // Unix (Linux)

    m_func = [this, boundFunc]()
    {
        try
        {
            if constexpr (std::is_void_v<ReturnType>)
            {
                boundFunc();
                m_promise->set_value({});
            }
            else
            {
                m_promise->set_value(boundFunc());
            }
        }
        catch (...)
        {
            m_promise->set_exception(std::current_exception());
        }
    };

    if (pthread_attr_init(&m_attr) != 0)
    {
        throw std::runtime_error("Failed to initialize thread attributes: " + std::string(strerror(errno)));
    }

    // Try to set thread properties
    SetPolicy(properties);
    SetPriority(properties);
    SetAffinity(properties);

    // Ensure the scheduling policy and priority are applied
    if (pthread_attr_setinheritsched(&m_attr, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        throw std::runtime_error("Failed to set inherit scheduler attribute: " + std::string(strerror(errno)));
    }


    auto m_funcptr = new std::function<void()>(std::move(m_func));

    // POSIX-specific thread creation
    if (pthread_create(&m_handle, nullptr, threadFuncWrapper, m_funcptr) != 0)
    {
        throw std::runtime_error("Failed to create thread: " + std::string(strerror(errno)));
    }
    m_initialized = true;

#endif
}


thread::thread(thread&& other) noexcept
    :
    m_handle(other.m_handle),
    m_func(std::move(other.m_func)),
    m_promise(std::move(other.m_promise)),
    m_future(std::move(other.m_future))
{
#ifdef _WIN32
    other.m_handle = nullptr; // Reset the thread handle
#else
    other.m_handle = pthread_t(); // Reset the thread handle
#endif
}


thread& thread::operator=(thread&& other) noexcept
{
    if (this != &other)
    {
        if (joinable())
        {
            join();
        }

        m_handle = other.m_handle;
        m_func = std::move(other.m_func);
        m_promise = std::move(other.m_promise);
        m_future = std::move(other.m_future);
#ifdef _WIN32
        other.m_handle = nullptr;
#else
        other.m_handle = pthread_t(); // Reset other's thread handle
#endif
    }
    return *this;
}


void thread::join()
{
#ifdef _WIN32
    if (WaitForSingleObject(m_handle, INFINITE) == WAIT_FAILED)
    {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to join thread: " + std::to_string(static_cast<size_t>(GetLastError())));
    }
    CloseHandle(m_handle);
    m_handle = nullptr; // Reset the thread handle
#else
    
    if (pthread_join(m_handle, nullptr) != 0)
    {
        throw std::runtime_error("Failed to join thread:" + std::string(strerror(errno)));
    }
    m_handle = pthread_t(); // Reset the thread handle
#endif
}


void thread::detach()
{
#ifdef _WIN32   // Windows

    if (CloseHandle(m_handle) == FALSE)
    {
        throw std::runtime_error("Failed to close thread handle");
    }
    
    m_handle = nullptr; // Reset the thread handle


#else           // Linux

    if (pthread_detach(m_handle) != 0)
    {
        throw std::runtime_error("Failed to detach thread");
    }
    
    m_handle = pthread_t(); // Reset the thread handle
#endif
}


bool thread::joinable() const
{
    bool res;
#ifdef _WIN32
    res = m_handle != nullptr; // Reset the thread handle
#else   // Unix (Linux)
    res = m_handle != pthread_t();
#endif
    return res;
}


std::any thread::getResult()
{
    return m_future.get();
}












void thread::SetPriority(const thread::Properties& properties)
{
    if (properties.priority == DEFAULT_POLICY)
    {
        return; // If default priority - nothing to do (its already the default behaviour)
    }

#ifdef _WIN32
    // Set priority for Windows
    if (SetThreadPriority(m_handle, properties.priority) == FALSE)
    {
        throw std::runtime_error("Failed to set thread priority");
    }

#else   // Unix (Linux)

    // Set thread priority
    struct sched_param param;
    param.sched_priority = properties.priority;

    if (pthread_attr_setschedparam(&m_attr, &param) != 0)
    {
        throw std::runtime_error("Failed to set thread priority: " + std::string(strerror(errno)));
    }
#endif
}


void thread::SetPolicy(const thread::Properties& properties)
{
    if (properties.policy == DEFAULT_POLICY)
    {
        return; // If default policy - nothing to do (its already the default behaviour)
    }

#ifdef _WIN32
        // windows doesn't support setting policy
#else   // Linux
    
    if (pthread_attr_setschedpolicy(&m_attr, m_properties.policy) != 0)
    {
        throw std::runtime_error("Failed to set thread policy: " + std::string(strerror(errno)));
    }
#endif
}

void thread::SetAffinity(const thread::Properties& properties)
{
    size_t coresCnt = 0;

    if (!m_properties.affinity.empty())
    {

#ifdef _WIN32

        DWORD_PTR mask = 0;

        for (size_t i = 0; i < m_properties.affinity.size(); ++i)
        {
            if (m_properties.affinity[i])
            {
                mask |= (static_cast<DWORD_PTR>(1) << i);
                ++coresCnt;
            }
        }

        if( coresCnt == m_properties.affinity.size() )
        {
            return; // If all cores selected - nothing to do (its already the default behaviour)
        }

        SetThreadAffinityMask(m_handle, mask);


#else // Linux

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        for (size_t i = 0; i < m_properties.affinity.size(); ++i)
        {
            if (m_properties.affinity[i])
            {
                CPU_SET(i, &cpuset);
                ++coresCnt;
            }
        }

        if( coresCnt == m_properties.affinity.size() )
        {
            return; // If all cores selected - nothing to do (its already the default behaviour)
        }

        if (pthread_attr_setaffinity_np(&m_attr, sizeof(cpu_set_t), &cpuset) != 0)
        {
            throw std::runtime_error("Failed to set thread affinity(CPU cores): " + std::string(strerror(errno)));
        }

#endif
    }
}







}


#endif //__thread__