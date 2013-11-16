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

    bool m_isRunning;

    Internal::ProcessBufferCaller<_BufferHandler, buffer> m_processBufferCaller;

    void threadLoop()
    {
      m_isRunning = true;
      Internal::callInitializeThread(m_handler);
      while (m_isRunning)
      {
        buffer & buffer = m_bufferQueue.beginReading();
        m_processBufferCaller.call(m_handler, buffer);
        m_bufferQueue.finishReading();
      }
      Internal::callUninitializeThread(m_handler);
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

      void start()
      {
        if (m_isRunning)
        {
          throw Exception("The thread is already running.");
        }
        m_isRunning = true;
        m_thread = boost::thread(boost::bind(&ReaderThread::threadLoop, this));
      }

      void stop()
      {
        m_isRunning = false;
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
  };
}

