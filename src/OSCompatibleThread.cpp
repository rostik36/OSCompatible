#include "OSCompatibleThread.h"
#include <iostream>

#ifdef _WIN32
#else
#include <sched.h>
#include <errno.h>
#include <cstring>
#endif


OSCompatibleThread::OSCompatibleThread()
    :
    m_thread(),
    m_attributes(),
    m_id(-1),
    //m_wrapperData(),
    m_errMsg(),
    m_isInitialized(false)
{ }



OSCompatibleThread::~OSCompatibleThread()
{
    if(m_isInitialized)
    {
        //pthread_attr_destroy(&m_attributes);
        //std::cout << "destructor" << std::endl;
        //FreeNDestroy();
    }
}

#ifdef _WIN32

OSCompatibleThread::ReturnStatus OSCompatibleThread::Init(os_thread_func_t func,
                                    void* functionArguments,
                                    int priority,
                                    int policy,
                                    const std::vector<bool>& cores)
{
    DWORD threadId;

    m_thread = CreateThread(
        NULL,          // Default security attributes
        0,             // Default stack size
        MyThreadFunction, // Thread function
        this,          // Parameter to the thread function ( this OSCompatibleThread instance )
        0,             // Default creation flags ( The thread runs immediately after creation )
        &threadId      // Pointer to the variable that receives the thread identifier
    );

    if(m_thread == NULL)
    {
        "Error creating thread";
        return OSCompatibleThread::ReturnStatus::ERR;
    }

    m_id = threadId;

    if(SetPriority(priority))
    {
        "Error setting thread priority";
        return OSCompatibleThread::ReturnStatus::FAILED_SET_PRIORITY;
    }

    if(policy(policy))
    {
        "Error setting thread policy";
        return OSCompatibleThread::ReturnStatus::FAILED_SET_POLICY;
    }

    if(SetCPUcores(cores))
    {
        "Error setting thread affinity";
        return OSCompatibleThread::ReturnStatus::FAILED_SET_CPU_CORES;
    }

    m_isInitialized = true;
    return OSCompatibleThread::ReturnStatus::SUCCESS;
}




int OSCompatibleThread::GetId() const
{
    return m_id;
}



#else // Linux





OSCompatibleThread::ReturnStatus OSCompatibleThread::Init(os_thread_func_t func,
                                    void* funcArgs,
                                    int priority,
                                    int policy,
                                    const std::vector<bool>& cores)
{
    int status;

    if(m_isInitialized)
    {
        LoadErrMsg("Error thread already initialized");
        return OSCompatibleThread::ReturnStatus::FAILED_THREAD_ALREADY_INITIALIZED;
    }
    std::cout << "Creating thread" << std::endl;

    if(pthread_attr_init(&m_attributes) != 0)
    {
        LoadErrMsg("Error initializing thread attributes");
        return OSCompatibleThread::ReturnStatus::FAILED_INITIALIZE_THREAD;
    }

    // Set the scheduling parameter inheritance attribute to EXPLICIT
    // PTHREAD_INHERIT_SCHED: The thread inherits scheduling parameters from the parent thread.
    // PTHREAD_EXPLICIT_SCHED: Scheduling parameters must be explicitly set before creating the thread.
    if (pthread_attr_setinheritsched(&m_attributes, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        LoadErrMsg("Error setting thread inheritance attribute");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_INHERIT_SCHED;
    }

    if(SetPolicy(policy))
    {
        LoadErrMsg("Error setting thread policy");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_POLICY;
    }

    if(SetPriority(priority))
    {
        LoadErrMsg("Error setting thread priority");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_PRIORITY;
    }

    if(SetCPUcores(cores))
    {
        return OSCompatibleThread::ReturnStatus::FAILED_SET_CPU_CORES;
    }

    if( (status = pthread_create(&m_thread, &m_attributes, func, funcArgs)) != 0)
    {
        if(status == EPERM)
        {
            LoadErrMsg("Error creating thread, No permission to set the scheduling policy run with permission.");
        }
        else
        {
            LoadErrMsg(std::string("Error creating thread, status: ") + std::string(strerror(status)));
        }
        return OSCompatibleThread::ReturnStatus::FAILED_INITIALIZE_THREAD;
    }

    m_isInitialized = true;

    return OSCompatibleThread::ReturnStatus::SUCCESS;
}



OSCompatibleThread::ReturnStatus OSCompatibleThread::Join(void* returnValue)
{
    int status;

    if(!m_isInitialized)
    {
        LoadErrMsg("Error thread not initialized");
        return OSCompatibleThread::ReturnStatus::FAILED_THREAD_NOT_INITIALIZED;
    }

    if(returnValue)
    {
        status = pthread_join(m_thread, &returnValue);
    }
    else
    {
        status = pthread_join(m_thread, NULL);
    }

    if(status != 0)
    {
        LoadErrMsg("Error joining thread");
        return OSCompatibleThread::ReturnStatus::FAILED_JOIN_THREAD;
    }

    return FreeNDestroy();
}
;

/* -------------------- Private functions implementations ------------------- */

// static void* ThreadWrapperFunction(void* this_)
// {
//     // OSCompatibleThread* thisObj = static_cast<OSCompatibleThread*>(this_);
//     // return thisObj->Run();
//     return 0;
// }



OSCompatibleThread::ReturnStatus OSCompatibleThread::SetPolicy(int policy)
{
    if(pthread_attr_setschedpolicy(&m_attributes, policy) != 0)
    {
        LoadErrMsg("Error setting thread policy");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_POLICY;
    }

    return OSCompatibleThread::ReturnStatus::SUCCESS;
}

OSCompatibleThread::ReturnStatus OSCompatibleThread::SetPriority(int priority)
{
    struct sched_param param;
    param.sched_priority = priority;

    if(pthread_attr_setschedparam(&m_attributes, &param) != 0)
    {
        LoadErrMsg("Error setting thread priority");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_PRIORITY;
    }

    return OSCompatibleThread::ReturnStatus::SUCCESS;
}

OSCompatibleThread::ReturnStatus OSCompatibleThread::SetCPUcores(const std::vector<bool>& cores)
{
    size_t cntCores = 0;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    for(size_t i = 0; i < cores.size(); ++i)
    {
        if(cores[i])
        {
            CPU_SET(i, &cpuset);
            ++cntCores;
        }
    }

    if(0 == cntCores)
    {
        LoadErrMsg("Error received no CPU cores to bind to the thread");
        return OSCompatibleThread::ReturnStatus::FAILED_NO_CPU_CORES_FLAGGED;
    }

    if(pthread_attr_setaffinity_np(&m_attributes, sizeof(cpu_set_t), &cpuset) != 0)
    {
        LoadErrMsg("Error setting thread affinity");
        return OSCompatibleThread::ReturnStatus::FAILED_SET_CPU_CORES;
    }

    return OSCompatibleThread::ReturnStatus::SUCCESS;
}

OSCompatibleThread::ReturnStatus OSCompatibleThread::FreeNDestroy()
{
    // !NOTE: maybe this is not needed this function is private and called in Join() only for now.
    // if(!m_isInitialized)
    // {
    //     "Error thread not initialized";
    //     return OSCompatibleThread::ReturnStatus::FAILED_THREAD_NOT_INITIALIZED;
    // }

    if(pthread_attr_destroy(&m_attributes)!= 0)
    {
        LoadErrMsg("Error destroying thread attributes");
        return OSCompatibleThread::ReturnStatus::FAILED_FREE_RESOURCES;
    }

    // int status;
    // if( (status = pthread_detach(m_thread)) != 0 )
    // {
    //     // EINVAL thread is not a joinable thread.
    //     // ESRCH  No thread with the ID thread could be found.
    //     LoadErrMsg(std::string("Error detaching thread, status: ") + std::to_string(status));
    //     return OSCompatibleThread::ReturnStatus::FAILED_FREE_RESOURCES;
    // }

    m_thread = 0;
    m_isInitialized = false;

    return OSCompatibleThread::ReturnStatus::SUCCESS;
}


char* OSCompatibleThread::GetErrMsg() const
{
    return const_cast<char*>(m_errMsg);
}


int OSCompatibleThread::GetId() const
{
    return m_id;
}

void OSCompatibleThread::LoadErrMsg(const std::string& errMsg)
{
    snprintf(m_errMsg, sizeof(m_errMsg), "%s, errno: %d, %s", errMsg.data(), errno, strerror(errno));
}



#endif