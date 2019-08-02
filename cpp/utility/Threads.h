#ifndef Threading_h
#define Threading_h

#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <cmath>
#include <thread>
#include <cambrian.h>

#include "Diagnostics.h"
#include "CommonUtility.h"

typedef enum CBThreadPriority
{
    CBThreadPriorityHighest = 0,
    CBThreadPriorityHigh,
    CBThreadPriorityModerate,
    CBThreadPriorityDefault,
    CBThreadPriorityLow,
    CBThreadPriorityLowest,
} CBThreadPriority;

class DLL_LOCAL CBMutex {
public:
    friend class CBCondition;
    CBMutex();
    ~CBMutex();
    
    bool isLocked() { return m_isLocked; };
    int64_t usecLockDuration();
    
    void lock();
    void unlock();
private:
    
    pthread_mutex_t m_mutex;
    bool m_isLocked = false;
    int64_t m_lockTime = 0;
};

class DLL_LOCAL CBCondition {
public:
    CBCondition();
    ~CBCondition();
    
    void signal();
    void wait(CBMutex& mutex);
    void timedWait(CBMutex& mutex, int delay_ms);
private:
    pthread_cond_t m_cond;
};


class DLL_LOCAL CBThread {
public:
    CBThread(std::string name);
    
    void start();
    
    virtual void run() = 0;
    
    int join();
    
    static long getID();
    std::string getThreadName();
    
    static std::vector<pthread_t> getRunningThreads();
    static std::string getThreadName(pthread_t thread);
    
    bool isRunning() { return m_isRunning;};
    
    static void setPThreadPriority(pthread_t thread, CBThreadPriority priority);
    
    static void listRunningCBThreads();
    
    bool m_changedPriority = false;
    
    CBThreadPriority getPriority() const {
        return m_priority;
    }
    
    void setPriority(CBThreadPriority priority) {
        m_priority = priority;
        m_hasSetPriority = true;
        m_changedPriority = true;
    }
    
protected:
    std::string m_thread_name;
    bool m_isRunning = false;
    virtual void tick() {};
    virtual ~CBThread() = 0;
    pthread_t m_thr;
    CBThreadPriority m_priority = CBThreadPriorityDefault;
    bool m_hasSetPriority = false;
private:
    CBThread();
    
    void pthreadSetName();
    
    static void* mainThread(void* thread);
    
    int64_t m_lastCPULogTime = 0;
};

template <class T> class DLL_LOCAL QueueThread : public CBThread {
public:
    QueueThread(std::string name, int queueSize) : CBThread(name), m_queueSize(queueSize)
    {
        
    }
    
    virtual void abort() {
        m_isAborted = true;
        m_frameMutex.lock();
        //std::queue<T> empty;
        //std::swap(m_frameQueue, empty);
        m_frameCondition.signal();
        m_frameMutex.unlock();
    }
    
    bool isAborted() {
        return m_isAborted;
    }
    
    virtual void addItem(T item) {
        if (!m_isRunning || m_isAborted) return;
        
        m_frameMutex.lock();
        
        while (m_frameQueue.size() >= m_queueSize) {
            m_frameQueue.pop();
        }
        
        m_frameQueue.push(item);
        
        if (m_priority == CBThreadPriorityHigh || m_wakeupTriggered || !m_hasRun) {
            m_wakeupTriggered = false;
            m_frameCondition.signal();
            m_hasRun = true;
        }
        
        m_frameMutex.unlock();
    }
    
    virtual void wakeup() {
        m_wakeupTriggered = true;
    }
    
    virtual void flush() {
        m_frameMutex.lock();
        std::queue<T> empty;
        std::swap(m_frameQueue, empty);
        m_frameMutex.unlock();
    }
    
    void exit() {
        if (m_frameMutex.isLocked()) m_frameMutex.unlock();
    }
    
protected:
    virtual ~QueueThread() {
        flush();
    }
    
    bool m_hasRun = false;
    
    virtual void run() {
        
        m_isRunning = true;
        
        while (!m_isAborted) {
            
            m_frameMutex.lock();
            m_frameCondition.timedWait(m_frameMutex, 10 + 100 * int(1 + m_priority));
            
            if (m_changedPriority) {
                m_changedPriority = false;
                CBThread::setPThreadPriority(m_thr, m_priority);
            }

            if (m_frameQueue.size() && !m_isAborted) {
                T item = m_frameQueue.front();
                m_frameQueue.pop();
                m_frameMutex.unlock();
                
                tick();
                
                if (!m_isAborted) {
                    handleItem(item);
                }
            } else {
                m_frameMutex.unlock();
            }
        }
        
        m_isRunning = false;
    }
    
    virtual bool handleItem(T item) = 0;
    
    bool m_isAborted = false;
    bool m_wakeupTriggered = false;
    CBCondition m_frameCondition;
    CBMutex m_frameMutex;
    int m_queueSize = 1;
        
    std::queue<T>m_frameQueue;
};

#endif
