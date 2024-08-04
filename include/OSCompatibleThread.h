/**
 * @file OSCompatibleThread.h
 * 
 * @brief Class to manage OS-compatible threads with priority, policy, and CPU 
 * core assignment.
 * 
 * !NOTE: Don't allocate many OSCompatibleThread on the stack, stack can run out 
 * !NOTE: and you will receive segmentation fault or some unexpected behavior.
 *
 * This class provides a platform-independent interface for managing threads, 
 * allowing the user to set thread priority, scheduling policy, and CPU core 
 * assignment. It supports both Windows and POSIX operating systems(Linux).
 *
 * The class provides methods for initializing and starting a new thread, waiting 
 * for the thread to finish, retrieving the thread's return value, setting thread 
 * priority, scheduling policy, and CPU cores, and retrieving the thread's unique 
 * identifier.
 *
 * The class also handles error cases and provides a method to retrieve the last 
 * error message.
 * 
 * 
 * @author Rostik
 * @version 0.1
 * @date 2024-07-27
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __OS_COMPATIBLE_THREAD__
#define __OS_COMPATIBLE_THREAD__

#include <functional>
#include <vector>
#include <string>

#include <cstring> // for strerror

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
#define HANDLE                  os_thread_t;
#define int                     os_thread_attributes_t; // in windows not actually used
#else
typedef pthread_t               os_thread_t;
typedef pthread_attr_t          os_thread_attributes_t;
#endif


typedef void* (*os_thread_func_t)(void*); // type of function to be executed by the thread



class OSCompatibleThread
{
public:
    /**
     * @brief Enumeration of possible return statuses for thread operations.
     */
    enum ReturnStatus {
        SUCCESS = 0,
        FAILED_SET_PRIORITY,
        FAILED_SET_POLICY,
        FAILED_SET_INHERIT_SCHED,
        FAILED_SET_CPU_CORES,
        FAILED_INITIALIZE_THREAD,
        FAILED_JOIN_THREAD,
        FAILED_WAIT_TIMEOUT,
        FAILED_UNEXPECTED_ERROR,
        FAILED_FREE_RESOURCES,
        FAILED_NO_CPU_CORES_FLAGGED,
        FAILED_THREAD_ALREADY_INITIALIZED,
        FAILED_THREAD_NOT_INITIALIZED,
    };

    OSCompatibleThread();
    ~OSCompatibleThread();
    /**
     * @brief Deleted copy constructor.
     */
    OSCompatibleThread(const OSCompatibleThread& o_) = delete;
    /**
     * @brief Deleted assignment operator.
     */
    OSCompatibleThread& operator=(const OSCompatibleThread& o_) = delete;


    /**
     * @brief Initialize and starts! the thread with the given function, 
     * priority, policy, and CPU cores.
     * 
     * !NOTE: Windows does not support Policy, so policy will be ignored and not effect nothing.
     * 
     * !NOTE: Linux will need privileges to set priority, policy and CPU cores
     * !NOTE: otherwise will return error about Operation not permitted. (use sudo)
     * 
     * !NOTE: Don't allocate many OSCompatibleThread on the stack, stack can run out 
     * !NOTE: and you will receive segmentation fault or some unexpected behavior.
     * 
     * @param func The function to be executed in the new thread.
     * @param funcArguments The arguments to be passed to the function.
     * @param priority The priority of the thread. Higher values indicate higher 
     * priority.
     * @param policy The scheduling policy of the thread.
     * @param cores A vector of boolean values indicating the CPU cores to be 
     * assigned to the thread.
     *
     * @return ReturnStatus indicating the success or failure of initializing the
     *  thread.
     *         - SUCCESS: The thread was successfully initialized.
     *         - FAILED_SET_PRIORITY: Failed to set the priority of the thread.
     *         - FAILED_SET_POLICY: Failed to set the scheduling policy of the thread.
     *         - FAILED_SET_CPU_CORES: Failed to set the CPU cores for the thread.
     *         - FAILED_INITIALIZED_THREAD: Failed to initialize the thread.
     *         - FAILED_NO_CPU_CORES_FLAGGED: No CPU cores were flagged for assignment.
     *         - FAILED_THREAD_ALREADY_INITIALIZED: The thread was already initialized and started.
     */
    template<typename Callable, typename... Args>
    ReturnStatus Init(int priority, int policy,
                    const std::vector<bool>& cores,
                    Callable&& func, Args&&... args);

    /**
     * @brief Wait for the thread to finish and retrieve its return value.
     *
     * @param returnValue A pointer to store the return value of the thread.
     *
     * @return ReturnStatus indicating the success or failure of joining the 
     * thread.
     */
    template <typename T>
    ReturnStatus Join(T* returnValue);

    ReturnStatus Join() {
        int status;

        if (!m_isInitialized) {
            LoadErrMsg("Error thread not initialized");
            return ReturnStatus::FAILED_THREAD_NOT_INITIALIZED;
        }

        status = pthread_join(m_thread, nullptr);

        if (status != 0) {
            LoadErrMsg("Error joining thread");
            return ReturnStatus::FAILED_JOIN_THREAD;
        }

        return FreeNDestroy();
    }

    /**
     * @brief Wait for the thread to finish or timeout, and retrieve its return 
     * value.
     *
     * @param returnValue A pointer to store the return value of the thread.
     * @param milliseconds The maximum time to wait for the thread to finish in 
     * milliseconds.
     *
     * @return ReturnStatus indicating the success or failure of joining the 
     * thread.
     */
    ReturnStatus JoinWithTimeout(void* returnValue, size_t milliseconds);

    /**
     * @brief Retrieve the last error message.
     *
     * @return A pointer to the last error message.
     */
    char* GetErrMsg() const;


    /**
     * @brief Retrieve the unique identifier of the thread.
     *
     * The unique identifier is a system-specific handle or value that uniquely 
     * identifies the thread within the operating system.
     *
     * @return The unique identifier of the thread.
     *         - A positive integer value for successful retrieval.
     *         - -1 for an error, in which case the error message can be retrieved 
     *           using the GetErrMsg() function.
     */
    int GetId() const;



private:
    /**
     * @brief A wrapper function for the user-provided thread function to be 
     * executed in the new thread.
     *
     * @param this_ A pointer to the OSCompatibleThread object.
     *
     * @return The return value of the user-provided thread function.
     */
    static void* ThreadWrapperFunction(void* this_);

    /**
     * @brief Set the priority of the thread.
     *
     * @param priority The priority of the thread.
     *
     * @return ReturnStatus indicating the success or failure of setting the 
     * priority.
     */
    ReturnStatus SetPriority(int priority);

    /**
     * @brief Set the scheduling policy of the thread.
     *
     * @param policy The scheduling policy of the thread.
     *
     * @return ReturnStatus indicating the success or failure of setting the 
     * policy.
     */
    ReturnStatus SetPolicy(int policy);

    /**
     * @brief Set the CPU cores to be assigned to the thread, so the thread will
     * run only on the specified cores.
     *
     * @param cores A vector of boolean values indicating the CPU cores to be 
     * assigned to the thread and the thread will run on them.
     *
     * @return ReturnStatus indicating the success or failure of setting the 
     * CPU cores.
     */
    ReturnStatus SetCPUcores(const std::vector<bool>& cores);


    /**
     * @brief Frees and destroys the thread resources.
     *
     * This function is responsible for freeing and destroying the thread resources 
     * such as thread handle, attributes, and error message. It should be called 
     * after the thread has finished its execution to ensure proper cleanup.
     *
     * @return ReturnStatus indicating the success or failure of freeing and 
     * destroying the thread resources.
     *         - SUCCESS: The thread resources were successfully freed and destroyed.
     *         - FAILED_FREE_RESOURCES: Failed to free the thread resources.
     *         - FAILED_THREAD_NOT_INITIALIZED: The thread was not initialized.
     */
    ReturnStatus FreeNDestroy();


    void LoadErrMsg(const std::string& errMsg);



    // Helper function to call the function with the correct arguments
    template <typename Callable, typename Tuple, size_t... I>
    static void callFuncImpl(Tuple* funcTuple, std::index_sequence<I...>) {
        std::get<0>(*funcTuple)(std::get<I + 1>(*funcTuple)...);
    }

    template <typename Ret, typename Callable, typename Tuple>
    static Ret callFunc(Tuple* funcTuple) {
        constexpr size_t size = std::tuple_size<Tuple>::value;
        return callFuncImpl<Ret, Callable>(funcTuple, std::make_index_sequence<size - 1>{});
    }






    // struct WrapperData {
    //     os_thread_func_t func;
    //     void* funcArgs;
    // };

    os_thread_t m_thread;
    os_thread_attributes_t m_attributes;
    // WrapperData m_wrapperData;
    // struct sched_param m_param;
    int m_id;
    char m_errMsg[128];
    bool m_isInitialized; // true if Init() was called successfully
};




// The implementation of the Init function template
template <typename Callable, typename... Args>
OSCompatibleThread::ReturnStatus OSCompatibleThread::Init(int priority, int policy, const std::vector<bool>& cores, Callable&& func, Args&&... args) {
    int status;

    if(m_isInitialized) {
        LoadErrMsg("Error thread already initialized");
        return ReturnStatus::FAILED_THREAD_ALREADY_INITIALIZED;
    }

    if(pthread_attr_init(&m_attributes) != 0) {
        LoadErrMsg("Error initializing thread attributes");
        return ReturnStatus::FAILED_INITIALIZE_THREAD;
    }

    if(pthread_attr_setinheritsched(&m_attributes, PTHREAD_EXPLICIT_SCHED) != 0) {
        LoadErrMsg("Error setting thread inheritance attribute");
        return ReturnStatus::FAILED_SET_INHERIT_SCHED;
    }

    if(SetPolicy(policy)) {
        LoadErrMsg("Error setting thread policy");
        return ReturnStatus::FAILED_SET_POLICY;
    }

    if(SetPriority(priority)) {
        LoadErrMsg("Error setting thread priority");
        return ReturnStatus::FAILED_SET_PRIORITY;
    }

    if(SetCPUcores(cores)) {
        return ReturnStatus::FAILED_SET_CPU_CORES;
    }

    using FuncTuple = std::tuple<Callable, Args...>;
    auto funcArgs = new FuncTuple(std::forward<Callable>(func), std::forward<Args>(args)...);

    auto lambda = [](void* arg) -> void* {
        auto funcArgsPtr = static_cast<FuncTuple*>(arg);
        void* ret = callFunc<void*, Callable>(funcArgsPtr);
        delete funcArgsPtr;
        return ret;
    };

    // auto lambda = [](void* arg) -> void* {
    //     auto funcArgsPtr = static_cast<FuncTuple*>(arg);
    //     auto& func = std::get<0>(*funcArgsPtr);
    //     auto result = std::apply([&func](auto&&... args) -> decltype(auto) {
    //         return func(std::forward<decltype(args)>(args)...);
    //     }, myTuple);
        
    //     delete funcArgsPtr;
        
    //     // To ensure the return type is handled properly, cast the result
    //     using ReturnType = decltype(result);
    //     ReturnType* returnValuePtr = new ReturnType(result);
    //     return static_cast<void*>(returnValuePtr);
    // };

    if((status = pthread_create(&m_thread, &m_attributes, lambda, funcArgs)) != 0) {
        if(status == EPERM) {
            LoadErrMsg("Error creating thread, No permission to set the scheduling policy run with permission.");
        } else {
            LoadErrMsg(std::string("Error creating thread, status: ") + std::string(strerror(status)));
        }
        delete funcArgs;
        return ReturnStatus::FAILED_INITIALIZE_THREAD;
    }

    m_isInitialized = true;
    return ReturnStatus::SUCCESS;
}



template <typename T>
OSCompatibleThread::ReturnStatus OSCompatibleThread::Join(T* returnValue)
{
    void* tempResult = nullptr;
    int status;

    if (!m_isInitialized) {
        LoadErrMsg("Error thread not initialized");
        return ReturnStatus::FAILED_THREAD_NOT_INITIALIZED;
    }

    status = pthread_join(m_thread, &tempResult);

    if (status != 0) {
        LoadErrMsg("Error joining thread");
        return ReturnStatus::FAILED_JOIN_THREAD;
    }

    if (returnValue) {
        *returnValue = *static_cast<T*>(tempResult);
    }

    return FreeNDestroy();
}










#endif //__OS_COMPATIBLE_THREAD__