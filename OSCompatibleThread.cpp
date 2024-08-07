#include "OSCompatibleThread.h"






OSCompatibleThread::OSCompatibleThread()
    : 
    m_thread(),
    m_func(nullptr),
    m_promise(std::make_shared<std::promise<std::any>>()),
    m_future(m_promise->get_future()),
    m_properties(DEFAULT_PROPERTIES)
{}

OSCompatibleThread::OSCompatibleThread(OSCompatibleThread&& other) noexcept
    :
    m_thread(other.thread_),
    m_func(std::move(other.m_func)),
    m_promise(std::move(other.m_promise)),
    m_future(std::move(other.m_future))
{
    other.thread_ = pthread_t();
}

OSCompatibleThread::OSCompatibleThread& 
OSCompatibleThread::operator=(OSCompatibleThread&& other) noexcept
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

void join()
{
    if (pthread_join(m_thread, nullptr) != 0)
    {
        throw std::runtime_error("Failed to join thread");
    }
    m_thread = pthread_t(); // Reset the thread handle
}

void OSCompatibleThread::detach()
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

bool OSCompatibleThread::joinable() const
{
    return m_thread != pthread_t();
}

std::any OSCompatibleThread::getResult()
{
    return m_future.get();
}
