#include "Threads.h"

#include <cstring>
#include <sys/time.h>
#include <iostream>
#include <stdint.h>
#include <map>
#include <cbar/CBAR_Common.hpp>

#ifdef __APPLE__
#   include <mach/mach.h>
#   include <mach/mach_init.h>
#   include <mach/thread_act.h>
#   include <mach/thread_info.h>
#else

#   define TH_USAGE_SCALE 1000
#endif

#define LOG_CPU_TIME 0
#define LOG_CPU_INTERVAL 2.0
#define LOG_MUTEX_AFTER 0.3

#if LOG_THREADS

#   define LOG_THREAD_COUNT 1

typedef struct thread_time {
    int64_t user_time_us;
    int64_t system_time_us;
    int64_t cpu_usage;
} thread_time_t;

thread_time_t thread_time();
thread_time_t thread_time_sub(thread_time_t const a, thread_time_t const b);

#endif

static CBMutex runningThreadsMutex;

typedef std::map<long, std::string> ThreadMap;
ThreadMap runningThreads;

CBMutex::CBMutex()
{
    pthread_mutex_init(&m_mutex, 0);
}

CBMutex::~CBMutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void
CBMutex::lock()
{
    m_lockTime = sys_usec_time();
    pthread_mutex_lock(&m_mutex);
    m_isLocked = true;
}

void
CBMutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
    m_isLocked = false;
    m_lockTime = 0;
}

int64_t CBMutex::usecLockDuration() {
    if (m_isLocked) {
        return sys_usec_time() - m_lockTime;
    }
    return 0;
}

CBCondition::CBCondition()
{
    pthread_cond_init(&m_cond, 0);
}


CBCondition::~CBCondition()
{
    pthread_cond_destroy(&m_cond);
}


void
CBCondition::signal()
{
    pthread_cond_signal(&m_cond);
}


void
CBCondition::wait(CBMutex& mutex)
{
    pthread_cond_wait(&m_cond, &mutex.m_mutex);
}

void
CBCondition::timedWait(CBMutex& mutex, int delay_ms)
{
    timeval now;
    gettimeofday(&now, 0);
    timespec t;
    t.tv_sec = now.tv_sec + (delay_ms / 1000);
    t.tv_nsec = (now.tv_usec*1000) + ((delay_ms % 1000) * 1000000);
    while (t.tv_nsec >= 1000000000) {
        t.tv_nsec -= 1000000000;
        t.tv_sec++;
    }
    pthread_cond_timedwait(&m_cond, &mutex.m_mutex, &t);
}

CBThread::CBThread()
 : CBThread("Undefined")
{
#if LOG_THREAD_LIFECYCLE
    CBLog( "cbthrd: Thread started with no name. Setting to 'Undefined'");
#endif
}

CBThread::CBThread(std::string name)
{
    memset(&m_thr, 0, sizeof(pthread_t));
    m_thread_name = name;
}

std::vector<pthread_t> CBThread::getRunningThreads() {
    // ...
    thread_act_array_t list;
    mach_msg_type_number_t thread_count = 0;
    
    std::vector<pthread_t> threads;
    
    const task_t    this_task = mach_task_self();
    const thread_t  this_thread = mach_thread_self();
    
    // 1. Get a list of all threads (with count):
    kern_return_t kr = task_threads(this_task, &list, &thread_count);
    
    if (kr != KERN_SUCCESS) {
        printf("error getting threads: %s", mach_error_string(kr));
        return threads;
    }
    
    for (int i=0; i<thread_count; i++) {
        pthread_t thread = pthread_from_mach_thread_np(list[i]);
        threads.push_back(thread);
    }
    
    mach_port_deallocate(this_task, this_thread);
    vm_deallocate(this_task, (vm_address_t)list, sizeof(thread_t) * thread_count);
    
    return threads;
}

void CBThread::pthreadSetName()
{
#if __APPLE__
    pthread_setname_np(m_thread_name.c_str());
#else
    pthread_setname_np(thr, thread_name.c_str());
#endif
}

CBThread::~CBThread() {
    
}

void
CBThread::start()
{
#if LOG_THREAD_LIFECYCLE
    CBLog("cbthrd: Starting thread %s", m_thread_name.c_str());
#endif
    pthread_create(&m_thr, 0, mainThread, this);
}

int
CBThread::join()
{
#if LOG_THREAD_LIFECYCLE
    CBLog("cbthrd: Joining thread %s", m_thread_name.c_str());
#endif
    return pthread_join(m_thr, 0);
}

long CBThread::getID() {
    return (long) pthread_self();
}

void*
CBThread::mainThread(void* thread)
{
    CBThread* th = (CBThread*)thread;
    
    auto threadName = th->m_thread_name;
    
#if LOG_THREAD_COUNT
    long threadID = getID();
    size_t numThreadsBefore;
    { // scope lock_guard
        std::lock_guard<CBMutex> lockGuard(runningThreadsMutex);
        runningThreads[threadID] = threadName;
        numThreadsBefore = runningThreads.size();
    }
    CBLog("cbthrd: Starting %s thread, now %lu threads running", threadName.c_str(), numThreadsBefore);
#endif
    
    th->m_isRunning = true;
    th->pthreadSetName();
    th->run();
    
#if LOG_THREADS
    size_t numThreadsAfter;
    { // scope lock_guard
        std::lock_guard<CBMutex> lockGuard(runningThreadsMutex);
        auto it = runningThreads.find(threadID);
        if (it != runningThreads.end()) {
            runningThreads.erase(it);
        }
        numThreadsAfter = runningThreads.size();
    }
    CBLog("cbthrd: STOPPING %s thread, now %lu threads running", threadName.c_str(), numThreadsAfter);
    if (!numThreadsAfter) CBLog("ALL THREADS STOPPED");
#endif
    
    return 0;
}

void CBThread::listRunningCBThreads() {
    std::lock_guard<CBMutex> lockGuard(runningThreadsMutex);
    
    CBLog("List of running threads:");
    
    if (runningThreads.size()) {
        for (const auto &it : runningThreads) {
            CBLog("%s \n", it.second.c_str());
        }
    } else {
        CBLog("NO THREADS RUNNING");
    }
    
}

std::string
CBThread::getThreadName() {
    return m_thread_name;
}

#if LOG_THREADS

#ifdef __APPLE__
// Convert time_value_t to microseconds.
int64_t time_value_to_us(time_value_t const t) {
    return (int64_t)t.seconds * 1e6 + t.microseconds;
}
#endif

thread_time_t thread_time_sub(thread_time_t const a, thread_time_t const b) {
    return (thread_time_t){
        .user_time_us   = a.user_time_us   - b.user_time_us,
        .system_time_us = a.system_time_us - b.system_time_us,
        .cpu_usage = a.cpu_usage - b.cpu_usage
    };
}

thread_time_t thread_time() {
#ifdef __APPLE__
    thread_basic_info_data_t basic_info;
    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    kern_return_t const result = thread_info(mach_thread_self(), THREAD_BASIC_INFO, (thread_info_t)&basic_info, &count);
    

    if (result == KERN_SUCCESS) {
        return (thread_time_t){
            .user_time_us   = time_value_to_us(basic_info.user_time),
            .system_time_us = time_value_to_us(basic_info.system_time),
            .cpu_usage = basic_info.cpu_usage
        };
    }
#else
    //TODO:android/linux
#endif
    return (thread_time_t){-1, -1, -1};
}

#endif

std::string CBThread::getThreadName(pthread_t thread) {
    char thread_name[256];
    pthread_getname_np(thread, thread_name, 256);
    
    return std::string(thread_name);
}

void CBThread::setPThreadPriority(pthread_t thread, CBThreadPriority priority) {
    struct sched_param params;
    int ret;
    
    auto thread_name = getThreadName(thread);
    
    // Now verify the change in thread priority
    int policy = 0;
    ret = pthread_getschedparam(thread, &policy, &params);
    if (ret != 0) {
        std::cout << "Couldn't retrieve real-time scheduling parameters for " << thread_name << std::endl;
        return;
    }
    
    // We'll set the priority to the maximum.
    int min = sched_get_priority_min(policy);
    int max = sched_get_priority_max(policy);
    int range = max - min;
    
    params.sched_priority = max - range * priority / int(CBThreadPriorityLowest);
    
    ret = pthread_setschedparam(thread, policy, &params);
    
    if (ret != 0) {
        // Print the error
        std::cout << "Unsuccessful in setting thread realtime priority for thread " << thread_name << std::endl;
        return;
    }
    
    // Print thread scheduling priority
    //std::cout << "Thread " << thread_name << " priority is " << params.sched_priority << std::endl;
}
