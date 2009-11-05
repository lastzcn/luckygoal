/**
 * @file   Thread.cpp
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Tue Aug  5 13:36:41 2008
 * 
 * @brief  
 * 
 * 
 */

//#define PTW32_STATIC_LIB
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include "Thread.h"


// **************** LastZ Check Leaks **********************************
// Define LASTZ_CHECKLEAKS macro in compile option to enable leak check.
// And put this code behind all #include in your .cpp file.
//#include "CheckLeaks.h"
// *********************************************************************

using namespace lastz;

// *********************************************************
// lastz::Mutex
// *********************************************************

namespace lastz{
     class MutexImpl
     {
     public:
          MutexImpl(){
               pthread_mutex_init(&m_mutex, NULL);
          }

          ~MutexImpl(){
               pthread_mutex_destroy(&m_mutex);
          }
          
          bool Enter(){
               if ( pthread_mutex_lock(&m_mutex) == 0 )
                    return true;
               else
                    return false;
          }
          
          bool Leave(){
               if ( pthread_mutex_unlock(&m_mutex) == 0 )
                    return true;
               else
                    return false;
          }

          bool TryEnter(){
               if ( pthread_mutex_trylock(&m_mutex) == 0 )
                    return true;
               else
                    return false;
          }
          
     private:
          pthread_mutex_t m_mutex;
     }; // class MutexImpl
} // namespace lastz

Mutex::Mutex(){
    m_pImpl = new MutexImpl();
}

Mutex::~Mutex(){
    if ( m_pImpl != NULL ){
        delete m_pImpl;
        m_pImpl = NULL;
    }
}
          
bool Mutex::Enter(){ return m_pImpl->Enter();}
bool Mutex::Leave(){ return m_pImpl->Leave();}
bool Mutex::TryEnter(){ return m_pImpl->TryEnter();}

// *********************************************************
// lastz::Condition
// *********************************************************

namespace lastz{
    
    class ConditionImpl
    {
    public:
        ConditionImpl(){
            m_bWait = false;
               
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_cond, NULL);

            pthread_mutex_lock(&m_mutex);
        }

        ~ConditionImpl(){
            pthread_mutex_unlock(&m_mutex);
               
            pthread_cond_destroy(&m_cond);
            pthread_mutex_destroy(&m_mutex);
        }
          
        bool IsWaiting(){
            int nRet = pthread_mutex_trylock(&m_mutex);
            if ( nRet == 0 ) {
                nRet = pthread_mutex_unlock(&m_mutex);
                return true;
            } else
                return false;
        }

        void Wait(bool bRightNow = false){
            if ( bRightNow || m_bWait ){
                m_bWait = false;
                pthread_cond_wait(&m_cond, &m_mutex);
            }
        }

        void TimedWait(int msec){
            struct timespec deltatime;
            int sec = msec / 1000;
            int nsec = (msec - sec * 1000) * 1000;
            deltatime.tv_sec = sec;
            deltatime.tv_nsec = nsec;
            //pthread_delay_np(&deltatime);

            int rv = pthread_cond_timedwait(&m_cond, &m_mutex, &deltatime);
            // switch ( rv ){
            // case ETIMEDOUT:
            //     // Handle timeout
            //     break;
            // case EINTR:
            //     // Interupted by signal
            //     break;
            // case EBUSY:
            // default:
            //     // Handle errors
            //     break;
            // case 0:
            //     // condition received a condition signal
            //     break;
            // }
        }
         
        bool Check(){
            if ( IsWaiting() ) return false;

            m_bWait = true;
            return true;
        }
          
        bool Done(){
            m_bWait = false;
            if ( !IsWaiting() ) {
                return false;
            }

            int nCondRet = pthread_cond_signal(&m_cond);
            if ( nCondRet != 0 ) return false;
            else
                return true;
        }
          
    private:
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;

        bool m_bWait;
          
    }; // class Condition

} // namespace lastz


Condition::Condition(){
    m_pImpl = new ConditionImpl();
}

Condition::~Condition(){
    if ( m_pImpl != NULL ){
        delete m_pImpl;
        m_pImpl = NULL;
    }
}

bool Condition::IsWaiting(){ return m_pImpl->IsWaiting();}
void Condition::Wait(bool bRightNow){ m_pImpl->Wait(bRightNow);}
bool Condition::Check(){ return m_pImpl->Check();}
bool Condition::Done(){ return m_pImpl->Done();}
void Condition::TimedWait(int msec){return m_pImpl->TimedWait(msec);};

// *********************************************************
// lastz::WorkingThread
// *********************************************************

namespace lastz{
    
    class WorkingThreadImpl
    {
    public:
        enum eThreadStatus
        {
            SUSPENDED = 0,
            EXITED,
            RESUMED,
            QUITTED
        };

    public:
        WorkingThreadImpl();
        virtual ~WorkingThreadImpl();

        bool Start(bool bSuspend = false);
        void Stop();

        void run();
        void cleanup();
          
        static void* threadFunc(void* arg);
        static void CleanupCallback(void* arg);

        static void YieldThread();
        void SuspendThread(int msec = 0);
        bool ResumeThread();
        bool IsThreadSuspended();
        bool WaitingForSuspended();
        int QuitThread();
        void DelayThread(int msec);
        
    private:
        pthread_t m_thread;
        pthread_attr_t m_threadAttr;
        bool m_bThreadStarted;
        
        bool m_bForceExit;
        
        int m_nState;

        lastz::Condition m_condResume;
//        lastz::Condition m_condTimedWait;
        
    public:
        WorkingThread* m_pParentObject;
        
    }; // class WorkingThreadImpl
} // namespace WorkingThreadImpl


WorkingThreadImpl::WorkingThreadImpl()
{
    m_pParentObject = NULL;
    
    m_bThreadStarted = false;
    m_bForceExit = false;
    m_nState = QUITTED;
}

WorkingThreadImpl::~WorkingThreadImpl()
{
    m_nState = EXITED;
}

bool WorkingThreadImpl::WaitingForSuspended()
{
    while ( !IsThreadSuspended() ){
        YieldThread();
    }
    return true;
}

void WorkingThreadImpl::Stop()
{
    if ( m_bThreadStarted ){
        m_bForceExit = true;
        SuspendThread();
        WaitingForSuspended();
        ResumeThread();
        
        int result = 0;
        pthread_join(m_thread, (void**)&result);
        pthread_attr_destroy(&m_threadAttr);
    }
}

bool WorkingThreadImpl::Start(bool bSuspend)
{
    if ( m_bThreadStarted ) return true;

    if ( bSuspend ){
        SuspendThread();
    }
    
    pthread_attr_init(&m_threadAttr);
    sched_param param;
    pthread_attr_getschedparam(&m_threadAttr, &param);
    param.sched_priority = 0;//SCHED_MIN;
    pthread_attr_setschedparam(&m_threadAttr, &param);
    pthread_create(&m_thread, &m_threadAttr, threadFunc, (void*)this);

    m_bThreadStarted = true;

    if ( bSuspend ){
        WaitingForSuspended();
    }
    
    return true;
}

bool WorkingThreadImpl::IsThreadSuspended()
{
    return m_condResume.IsWaiting();
}

void WorkingThreadImpl::SuspendThread(int msec)
{
    if ( msec == 0 )
        m_condResume.Check();
    else{
        m_condResume.TimedWait(msec);
        //DelayThread(msec);
    }
}

bool WorkingThreadImpl::ResumeThread()
{
    return m_condResume.Done();
}

void WorkingThreadImpl::YieldThread()
{
    sched_yield();
}

int WorkingThreadImpl::QuitThread()
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int nRet = pthread_cancel(m_thread);
    m_nState = EXITED;
    return nRet;
}

void WorkingThreadImpl::DelayThread(int msec){
    // struct timespec deltatime;
    // int sec = msec / 1000;
    // int nsec = (msec - sec * 1000) * 1000;
    // deltatime.tv_sec = sec;
    // deltatime.tv_nsec = nsec;
    // pthread_delay_np(&deltatime);
    m_condResume.TimedWait(msec);
}

void* WorkingThreadImpl::threadFunc(void* arg){
    WorkingThreadImpl* pWorkingThreadImpl = (WorkingThreadImpl*)arg;
    
    pWorkingThreadImpl->run();
    
    pthread_exit(arg);
    
    return NULL;
}

void WorkingThreadImpl::CleanupCallback(void* arg)
{
    WorkingThreadImpl* pWorkingThreadImpl = (WorkingThreadImpl*)arg;
    pWorkingThreadImpl->cleanup();
}

void WorkingThreadImpl::cleanup()
{
    m_nState = QUITTED;
}

void WorkingThreadImpl::run()
{
    pthread_cleanup_push(&CleanupCallback, this);

    for ( ; ; ) {
        m_condResume.Wait();
        if ( m_bForceExit ) break;

        if ( m_pParentObject != NULL )
            m_pParentObject->threadLoop();

        YieldThread();        
    }
    
    pthread_cleanup_pop(0);
    
}

WorkingThread::WorkingThread()
{
    m_pImpl = new WorkingThreadImpl();
    m_pImpl->m_pParentObject = this;
}

WorkingThread::~WorkingThread()
{
    if ( m_pImpl != NULL ){
        delete m_pImpl;
        m_pImpl = NULL;
    }
}


bool WorkingThread::Start(bool bSuspend){ return m_pImpl->Start(bSuspend);};
void WorkingThread::Stop(){m_pImpl->Stop();};

void WorkingThread::YieldThread(){WorkingThreadImpl::YieldThread();};

void WorkingThread::SuspendThread(int msec){m_pImpl->SuspendThread(msec);};
bool WorkingThread::ResumeThread(){return m_pImpl->ResumeThread();};
bool WorkingThread::IsThreadSuspended(){return m_pImpl->IsThreadSuspended();};
bool WorkingThread::WaitingForSuspended(){return m_pImpl->WaitingForSuspended();};
int WorkingThread::QuitThread(){return m_pImpl->QuitThread();};
void WorkingThread::DelayThread(int msec){m_pImpl->DelayThread(msec);};
