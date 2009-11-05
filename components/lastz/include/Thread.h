/**
 * @file   Thread.h
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Tue Aug  5 13:35:41 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef __LASTZ_THREAD_H__
#define __LASTZ_THREAD_H__
#include "lastz.h"
#include "DateTime.h"

namespace lastz
{
     class MutexImpl;
     class LASTZ_API Mutex
     {
     public:
          Mutex();
          virtual ~Mutex();
          
          bool Enter();
          bool Leave();
          bool TryEnter();
          
     private:
          MutexImpl* m_pImpl;
     };

     class LASTZ_API ScopedLock
     {
     public:
     ScopedLock(Mutex& mutex) : m_mutex(mutex){
               m_mutex.Enter();
          }
          
          virtual ~ScopedLock(){
               m_mutex.Leave();
          }
          
     private:
          Mutex& m_mutex;
     }; // class ScopeLock

     class ConditionImpl;     
     class LASTZ_API Condition
     {
     public:
          Condition();
          virtual ~Condition();
          
          bool IsWaiting();
          void Wait(bool bRightNow = false);
          bool Check();
          bool Done();
          void TimedWait(int msec);
          
     private:
          ConditionImpl* m_pImpl;
     }; // class Condition

     class WorkingThreadImpl;     
     class LASTZ_API WorkingThread
     {
     public:
          WorkingThread();
          virtual ~WorkingThread();

          bool Start(bool bSuspend = false);
          void Stop();

          static void YieldThread();
          
          void SuspendThread(int msec = 0);
          bool ResumeThread();
          bool IsThreadSuspended();
          bool WaitingForSuspended();
          int QuitThread();
          void DelayThread(int msec);
          
          virtual void threadLoop() = 0;

     private:
          WorkingThreadImpl* m_pImpl;
        
     }; // class WorkingThread


     template<class TimerObject>
          class TimerThread : public WorkingThread
     {
     public:
          TimerThread(){
               m_bBegin = false;
               m_pUserData = NULL;
               m_interval = 1000;
               m_lastTime = DateTime::Now();
          }

          int GetInterval() const {return m_interval;};
          void SetInterval(int interval) {
               m_lastTime = DateTime::Now();
               m_interval = interval;
          };

          void* GetUserData() const {return m_pUserData;};
          void SetUserData(void* pUserData) {m_pUserData = pUserData;};

          TimerObject& GetTimerObject() {return m_timerObject;};
          const TimerObject& GetTimerObject() const {return m_timerObject;};
          
          void Begin(){
               m_bBegin = true;
               Start(true);
               m_lastTime = DateTime::Now();
               ResumeThread();
          }
          
          void End(){
               if ( m_bBegin ){
                    SuspendThread();
                    WaitingForSuspended();
               }
               m_bBegin = false;
          }
          
     protected:
          virtual void threadLoop(){
               DateTime now = DateTime::Now();
               Time t = DateTime::Duration(m_lastTime, now);
               if ( t.TotalMicroSeconds() >= m_interval ){
                    if ( m_interval > 0 ){
                         m_timerObject(m_pUserData);
                    }
                    m_lastTime = now;
               }
               if ( m_interval < 1000 ){
                    if ( m_interval > 0 ) 
                         DelayThread(m_interval / 2);
                    else
                         DelayThread(1000);
               } else
                    DelayThread(1000);
          }

     private:
          int m_interval;
          DateTime m_lastTime;
          void* m_pUserData;
          TimerObject m_timerObject;
          bool m_bBegin;
          
     }; // class TimerThread
          
} // namespace lastz

#endif // __LASTZ_THREAD_H__

