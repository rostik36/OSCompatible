#include <iostream>
#include <pthread.h>
#include <functional>
#include <future>
#include <any>

class OSCompatibleThread
{
public:

    /**
     * @brief Default constructor for the OSCompatibleThread class.
     *
     * Initializes the OSCompatibleThread object with default values.
     * The m_thread handle is set to pthread_t(), indicating that the thread is 
     * not joinable.
     * The m_func pointer is set to nullptr, indicating that no function is 
     * associated with the thread.
     * A new std::promise and std::future are created to handle the return 
     * value (if any) from the thread function.
     * The std::promise is stored in a shared_ptr, and the std::future is 
     * stored as a member variable.
     *
     * @note The default constructed OSCompatibleThread object is not joinable.
     * @note The m_func pointer is set to nullptr, indicating that no function i
     * s associated with the thread.
     * @note The std::promise and std::future are used to handle the return 
     * value (if any) from the thread function.
     */
    OSCompatibleThread(): 
            m_thread(),
            m_func(nullptr),
            m_promise(std::make_shared<std::promise<std::any>>()),
            m_future(m_promise->get_future())
        {}


    /**
     * @brief Constructor for the OSCompatibleThread class that takes a function
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
    OSCompatibleThread(Function&& func, Args&&... args) : OSCompatibleThread()
    {
        using ReturnType = std::invoke_result_t<Function, Args...>;

        auto boundFunc = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);

        m_func = [this, boundFunc]() {
            try
            {
                if constexpr (std::is_void_v<ReturnType>)
                {
                    boundFunc();
                    m_promise->set_value(std::any{});
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

        if (m_func)
        {
            auto m_funcptr = new std::function<void()>(std::move(m_func));
            if (pthread_create(&thread_, nullptr, threadFuncWrapper, m_funcptr) != 0)
            {
                delete m_funcptr; // Clean up in case of error
                throw std::runtime_error("Failed to create thread");
            }
        }
        else
        {
            throw std::runtime_error("No function to execute in thread");
        }
    }

    // Deleting copy constructor and assignment operator
    OSCompatibleThread(const OSCompatibleThread&) = delete;
    OSCompatibleThread& operator=(const OSCompatibleThread&) = delete;

    /**
     * @brief Move constructor for the OSCompatibleThread class.
     * This constructor moves the resources from another OSCompatibleThread object 
     * to the newly created object.
     * After the move, the other object is left in a valid but unspecified state.
     *
     * @param other The other OSCompatibleThread object from which to move the resources.
     * @throws No exceptions are thrown by this constructor.
     */
    OSCompatibleThread(OSCompatibleThread&& other) noexcept
        :
        m_thread(other.thread_),
        m_func(std::move(other.m_func)),
        m_promise(std::move(other.m_promise)),
        m_future(std::move(other.m_future))
    {
        other.thread_ = pthread_t();
    }


    OSCompatibleThread& operator=(OSCompatibleThread&& other) noexcept
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
    void join()
    {
        if (pthread_join(m_thread, nullptr) != 0)
        {
            throw std::runtime_error("Failed to join thread");
        }
        m_thread = pthread_t(); // Reset the thread handle
    }



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
    void detach()
    {
        if (pthread_detach(m_thread) != 0)
        {
            throw std::runtime_error("Failed to detach thread");
        }
        m_thread = pthread_t(); // Reset the thread handle
    }

    
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
     * @note The default constructed OSCompatibleThread object is not joinable.
     */
    bool joinable() const
    {
        return m_thread != pthread_t();
    }


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
    std::any getResult()
    {
        return m_future.get();
    }

private:
    // Wrapper function to be passed to pthread_create
    static void* threadFuncWrapper(void* arg)
    {
        auto* func = static_cast<std::function<void()>*>(arg);
        (*func)();
        delete func;
        return nullptr;
    }

    pthread_t m_thread;
    std::function<void()> m_func;
    // used for the return value (if exists)
    std::shared_ptr<std::promise<std::any>> m_promise;
    std::future<std::any> m_future;
};






// Example function
int exampleFunction(int a, double b) {
    std::cout << "Function called with arguments: " << a << ", " << b << std::endl;
    return a + static_cast<int>(b);
}

void voidFunction(int a) {
    std::cout << "Void function called with argument: " << a << std::endl;
}

int main() {
    // Create and start the thread for a function with a return value
    OSCompatibleThread thread1(exampleFunction, 42, 3.14);
    thread1.start();
    thread1.join();

    // Retrieve the result
    std::any result = thread1.getResult();
    if (result.has_value()) {
        std::cout << "Result: " << std::any_cast<int>(result) << std::endl;
    }

    // Create and start the thread for a void function
    OSCompatibleThread thread2(voidFunction, 42);
    thread2.start();
    thread2.join();

    // No result to retrieve for void function
    std::any voidResult = thread2.getResult();
    if (!voidResult.has_value()) {
        std::cout << "Void function completed successfully" << std::endl;
    }

    return 0;
}
