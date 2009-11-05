/**
 * @file   ThreadStack.h
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Mon Oct 12 11:01:57 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef __THRADSTACK_H__
#define __THRADSTACK_H__

#include <stack>
#include <ucommon/thread.h>

template<class T, class TThread>
     class ThreadStack : public ucc::JoinableThread
{
public:
     ThreadStack(size_t nMaxThreads = 0){
          m_bForceExit = false;
          m_nMaxThreads = nMaxThreads;
     }
     
     virtual ~ThreadStack(){
     }
     

     virtual void run(){
          while ( !m_bForceExit ){
               m_mutex.lock();

               std::vector<ucc::JoinableThread*> cloneThreads;
               for ( size_t i = 0 ; i < m_runningThreads.size() ; i++ ){
                    ucc::JoinableThread* pJoinableThread = m_runningThreads[i];
                    if ( pJoinableThread->isRunning() ){
                         cloneThreads.push_back(pJoinableThread);
                    } else {
                         ucc::Thread* pThread = static_cast<ucc::Thread*>(pJoinableThread);
                         delete pThread;
                    }
               }
               m_runningThreads = cloneThreads;
               size_t nRunningThreads = m_runningThreads.size();
               
               while ( !m_stack.empty() && ( m_nMaxThreads == 0 || nRunningThreads < m_nMaxThreads ) ){
                    T t = m_stack.top();
                    m_stack.pop();
                    
                    ucc::JoinableThread* pThread = static_cast<ucc::JoinableThread*>(new TThread(t));
                    m_runningThreads.push_back(pThread);
                    pThread->start();
                    nRunningThreads++;
               }
               m_mutex.unlock();
               ucc::Thread::yield();
          }
     }
     
     virtual void exit(){
          ClearAll();
     }
     
     void Push(const T& t){
          m_mutex.lock();
          m_stack.push(t);
          m_mutex.unlock();
     }

     void ClearAll(){
          m_mutex.lock();
          while ( !m_stack.empty() ){
               m_stack.pop();
          }
          for ( size_t i = 0 ; i < m_runningThreads.size() ; i++ ){
               ucc::Thread* pThread = m_runningThreads[i];
               delete pThread;
          }
          
          m_mutex.unlock();
     }

     void ForceExit(){
          m_bForceExit = true;
     }
     
private:
     ucc::mutex m_mutex;
     std::stack<T> m_stack;
     size_t m_nMaxThreads;
     bool m_bForceExit;

     std::vector<ucc::JoinableThread*> m_runningThreads;
     
}; // class ThreadStack


#endif /* __THRADSTACK_H__ */
