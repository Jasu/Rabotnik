#pragma once

#include <Rabotnik/Internal/Utilities.h>
#include <Rabotnik/Exception.h>

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Rabotnik
{
  namespace Internal
  {
    HAS_MEMBER_FUNCTION(initializeThread);
    HAS_MEMBER_FUNCTION(uninitializeThread);
    HAS_MEMBER_FUNCTION(processBuffer);

    template<typename _BufferHandler>
    typename boost::enable_if<
      typename HasMemberFunction_initializeThread<
        _BufferHandler, 
        void (_BufferHandler::*)()
      >::type 
    >::type
    callInitializeThread(_BufferHandler & bufferHandler)
    {
      bufferHandler.initializeThread();
    }

    template<typename _BufferHandler>
    typename boost::disable_if<
      typename HasMemberFunction_initializeThread<
        _BufferHandler, 
        void (_BufferHandler::*)()
      >::type
    >::type
    callInitializeThread(_BufferHandler & bufferHandler)
    {
    }

    template<typename _BufferHandler>
    typename boost::enable_if<
      typename HasMemberFunction_uninitializeThread<
        _BufferHandler, 
        void (_BufferHandler::*)()
      >::type
    >::type
    callUninitializeThread(_BufferHandler & bufferHandler)
    {
      bufferHandler.uninitializeThread();
    }

    template<typename _BufferHandler>
    typename boost::disable_if<
      typename HasMemberFunction_uninitializeThread<
        _BufferHandler, 
        void (_BufferHandler::*)()
      >::type
    >::type
    callUninitializeThread(_BufferHandler & bufferHandler)
    {
    }

    template<typename _BufferHandler, typename _Buffer, typename _Enabler = void>
    class ProcessBufferCaller
    {
      public:
        void call(_BufferHandler & bufferHandler, _Buffer & buffer)
        {
          bufferHandler.processBuffer(buffer);
        }
    };

    template<typename _BufferHandler, typename _Buffer>
    class ProcessBufferCaller<
      _BufferHandler, 
      _Buffer,
      typename boost::enable_if<
        typename HasMemberFunction_processBuffer<
          _BufferHandler, 
      /**@todo Accept CV-qualifiers and non-reference for buffer */
          void (_BufferHandler::*)(_Buffer &, unsigned int)
        >::type
      >::type
    >
    {
      boost::posix_time::ptime m_lastTick;
      public:
        void call(_BufferHandler & bufferHandler, _Buffer & buffer)
        {
          boost::posix_time::ptime currentTime 
            = boost::posix_time::microsec_clock::universal_time();
          boost::posix_time::time_duration duration;
          if (!m_lastTick.is_not_a_date_time())
          {
            duration = currentTime - m_lastTick;
          }
          bufferHandler.processBuffer(buffer, duration.total_microseconds());
          m_lastTick = currentTime;
        }
    };
  }

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
  };
}

