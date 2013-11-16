#pragma once

#include <Rabotnik/Internal/Callers.h>
#include <Rabotnik/Exception.h>

#include <boost/bind.hpp>
//Hack to get around getpagesize() working on iOS SDK where it is deprecated
//and removed when _POSIX_SOURCE is defined. It is required by thread.hpp.
extern "C" { int getpagesize(); }

#include <boost/thread.hpp>

namespace Rabotnik
{

  enum ReaderThreadState {
    READER_THREAD_STATE_STOPPED,
    READER_THREAD_STATE_STARTING,
    READER_THREAD_STATE_RUNNING,
    READER_THREAD_STATE_STOPPING,
  };

  /**
   * @brief Thread which handles buffers.
   *
   * @param _BufferHandler
   *  Type to handle the events. Contained within the ReaderThread.
   *  Must have void processBuffer() or void processBuffer(unsigned int microsecondsFromLastCall).
   *  May have void initializeThread() and/or void uninitializeThread().
   */
  template<
    typename _BufferQueue, 
    typename _BufferHandler
  >
  class ReaderThread
  {
    typedef typename _BufferQueue::buffer buffer;
    boost::thread m_thread;

    _BufferQueue m_bufferQueue;

    _BufferHandler m_handler;

    ReaderThreadState m_state;

    mutable boost::mutex m_stateMutex;

    mutable boost::condition_variable m_stateCond;

    Internal::ProcessBufferCaller<_BufferHandler, buffer> m_processBufferCaller;

    void setState(ReaderThreadState state)
    {
      boost::unique_lock<boost::mutex> lock(m_stateMutex);
      m_state = state;
    }

    void threadLoop()
    {
      Internal::callInitializeThread(m_handler);
      setState(READER_THREAD_STATE_RUNNING);
      while (m_state == READER_THREAD_STATE_RUNNING)
      {
        buffer & buffer = m_bufferQueue.beginReading();
        m_processBufferCaller.call(m_handler, buffer);
        m_bufferQueue.finishReading();
      }
      Internal::callUninitializeThread(m_handler);
      setState(READER_THREAD_STATE_STOPPED);
    }

    /**
     * @brief Callable for boost::thread.
     */
    struct ThreadLoopCallable {
      ReaderThread * m_thread;
      ThreadLoopCallable(ReaderThread * thread)
        : m_thread(thread)
      {
      }

      void operator()() {
        m_thread->threadLoop();
      }
    };

    public:
      buffer & beginWriting() { return m_bufferQueue.beginWriting(); }
      void finishWriting() { m_bufferQueue.finishWriting(); }

      ReaderThread()
        : m_state(READER_THREAD_STATE_STOPPED)
      {
      }

      void start()
      {
        if (m_state != READER_THREAD_STATE_STOPPED)
        {
          throw Exception("The thread is not stopped.");
        }
        setState(READER_THREAD_STATE_STARTING);
        m_thread = boost::thread(boost::bind(&ReaderThread::threadLoop, this));
      }

      void stop()
      {
        setState(READER_THREAD_STATE_STOPPING);
      }

      void join()
      {
        m_thread.join();
      }

      _BufferHandler & getBufferHandler()
      {
        return m_handler;
      }

      const _BufferHandler & getBufferHandler() const
      {
        return m_handler;
      }

      void waitForState(ReaderThreadState state) const
      {
        if (m_state != state)
        {
          boost::unique_lock<boost::mutex> lock(m_stateMutex);
          while(m_state != state)
          {
            m_stateCond.wait(lock);
          }
        }
      }
  };
}

