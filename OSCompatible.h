/**
* @file Thread.h
* @author Rostik
* @brief 
* @version 0.1
* @date 2024-08-07
* 
* @copyright Copyright (c) 2024
* 
*/
#ifndef __OS_COMPATIBLE_THREAD__
#define __OS_COMPATIBLE_THREAD__

#include <functional>
#include <future>
#include <any>
#include "OSCompatible.hpp"

#ifdef _WIN32       // Windows
#include <windows.h>
#else               // Linux
#include <pthread.h>
#endif


namespace OSCompatible
{

// Define handle ThreadId type based on the operating system
#ifdef _WIN32       // Windows
typedef DWORD       ThreadId;
#else               // Linux
typedef pthread_t   ThreadId;
#endif


class Thread
{
public:
    /**
     * @brief Structure to hold thread properties such as priority, policy, and
     * CPU affinity (CPU cores to which the thread pinned and will be running on).
     */
    struct Properties
    {
        int priority;
        int policy;
        std::vector<bool> affinity; // CPU affinity (CPU cores to which the thread pinned and will be running on)
    }

    const int DEFAULT_PRIORITY = 255;
    const int DEFAULT_POLICY = 255;
    const std::vector<bool> DEFAULT_AFFINITY = {}; // No CPU affinity (thread will be running on all available CPU cores)
    const Properties DEFAULT_PROPERTIES = {DEFAULT_PRIORITY, DEFAULT_POLICY, DEFAULT_AFFINITY};


    /**
     * @brief Default constructor for the Thread class.
     *
     * Initializes the Thread object with default values.
     * The m_thread handle is set to pthread_t(), indicating that the thread is 
     * not joinable.
     * The m_func pointer is set to nullptr, indicating that no function is 
     * associated with the thread.
     * A new std::promise and std::future are created to handle the return 
     * value (if any) from the thread function.
     * The std::promise is stored in a shared_ptr, and the std::future is 
     * stored as a member variable.
     *
     * @note The default constructed Thread object is not joinable.
     * @note The m_func pointer is set to nullptr, indicating that no function i
     * s associated with the thread.
     * @note The std::promise and std::future are used to handle the return 
     * value (if any) from the thread function.
     */
    Thread();


    /**
     * @brief Constructor for the Thread class that takes a function
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
    template <typename Function, typename... Args>
    Thread(Function&& func, Args&&... args);


    /**
     * @brief Constructor for the Thread class that takes a function,
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
     */
    template <typename Function, typename... Args>
    Thread(const Properties& properties, Function&& func, Args&&... args);


    // Deleting copy constructor and assignment operator
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    /**
     * @brief Move constructor for the Thread class.
     * This constructor moves the resources from another Thread object 
     * to the newly created object.
     * After the move, the other object is left in a valid but unspecified state.
     *
     * @param other The other Thread object from which to move the resources.
     * @throws No exceptions are thrown by this constructor.
     */
    Thread(Thread&& other) noexcept;


    Thread& operator=(Thread&& other) noexcept;


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
     * @note The default constructed Thread object is not joinable.
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

    ThreadId m_thread; // Thread handle
    std::function<void()> m_func;
    // used for the return value (if exists)
    std::shared_ptr<std::promise<std::any>> m_promise;
    std::future<std::any> m_future;
    Properties m_properties; // Additional properties for the thread, if needed
};

} // namespace

#endif //__OS_COMPATIBLE_THREAD__