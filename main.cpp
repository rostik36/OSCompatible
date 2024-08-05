#include <iostream>
#include <pthread.h>
#include <memory>
#include <functional>
#include <future>

// Utility to convert any callable to void*(*)(void*)
template <typename ReturnType>
void* threadFuncWrapper(void* arg) {
    // Cast the argument back to a unique_ptr to manage its lifetime
    std::unique_ptr<std::packaged_task<ReturnType()>> task(static_cast<std::packaged_task<ReturnType()>*>(arg));
    // Execute the task
    (*task)();
    return nullptr;
}

template <typename Function, typename... Args>
class ThreadWrapper {
public:
    using ReturnType = std::result_of_t<Function(Args...)>;

    ThreadWrapper(Function&& func, Args&&... args)
        : task_(std::bind(std::forward<Function>(func), std::forward<Args>(args)...)) {
    }

    void start() {
        auto task_ptr = new std::packaged_task<ReturnType()>(std::move(task_));
        result_ = task_ptr->get_future();
        if (pthread_create(&thread_, nullptr, threadFuncWrapper<ReturnType>, task_ptr) != 0) {
            throw std::runtime_error("Failed to create thread");
        }
    }

    void join() {
        pthread_join(thread_, nullptr);
    }

    ReturnType getResult() {
        return result_.get();
    }

private:
    pthread_t thread_;
    std::packaged_task<ReturnType()> task_;
    std::future<ReturnType> result_;
};

// Helper function to create and start the thread
template <typename Function, typename... Args>
ThreadWrapper<Function, Args...> makeThread(Function&& func, Args&&... args) {
    return ThreadWrapper<Function, Args...>(std::forward<Function>(func), std::forward<Args>(args)...);
}

// Example function
int exampleFunction(int a, double b) {
    std::cout << "Function called with arguments: " << a << ", " << b << std::endl;
    return a + static_cast<int>(b);
}

int main() {
    // Create and start the thread
    auto thread = makeThread(exampleFunction, 42, 3.14);
    thread.start();
    thread.join();

    // Retrieve the result
    int result = thread.getResult();
    std::cout << "Result: " << result << std::endl;

    return 0;
}
