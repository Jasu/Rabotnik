#pragma once

#include <Rabotnik/Internal/Callers.h>
#include <Rabotnik/Internal/StateManager.h>
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
   *  Interruption pointsin processBuffer() may throw.
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

    Internal::StateManager m_stateManager;

    Internal::ProcessBufferCaller<_BufferHandler, buffer> m_processBufferCaller;

    void threadLoop()
    {
      Internal::callInitializeThread(m_handler);
      m_stateManager.setState(READER_STATE_RUNNING);
      while (m_stateManager.getState() == READER_STATE_RUNNING)
      {
        try 
        {
          buffer & buffer = m_bufferQueue.beginReading();
          {
            boost::this_thread::disable_interruption di;
            m_processBufferCaller.call(m_handler, buffer);
            m_bufferQueue.finishReading();
          }
        }
        catch (const boost::thread_interrupted & e)
        {
        }

      }
      Internal::callUninitializeThread(m_handler);
      m_stateManager.setState(READER_STATE_STOPPED);
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
        if (m_stateManager.getState() != READER_STATE_STOPPED)
        {
          throw Exception("The thread is not stopped.");
        }
        m_stateManager.setState(READER_STATE_STARTING);
        m_thread = boost::thread(boost::bind(&ReaderThread::threadLoop, this));
      }

      void stop()
      {
        m_stateManager.setState(READER_STATE_STOPPING);
      }


      void interrupt()
      {
        m_stateManager.setState(READER_STATE_STOPPING);
        m_thread.interrupt();
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

      void waitForState(ReaderState state) const
      {
        m_stateManager.waitForState(state);
      }

      /**
       * @todo Enable this CTOR with template magic iff handler has default 
       * ctor.
       */
      ReaderThread()
      {
      }

      /**
       * @param arg1 Passed tot the handler constructor.
       */
      template<typename _Arg1>
      ReaderThread(_Arg1 arg1) 
        : m_handler(arg1)
      {
      }

      /**
       * @param arg1 Passed tot the handler constructor.
       * @param arg2 Passed tot the handler constructor.
       */
      template<typename _Arg1, typename _Arg2>
      ReaderThread(_Arg1 arg1, _Arg2 arg2)
        : m_handler(arg1, arg2)
      {
      }

      /**
       * @param arg1 Passed tot the handler constructor.
       * @param arg2 Passed tot the handler constructor.
       * @param arg3 Passed tot the handler constructor.
       */
      template<typename _Arg1, typename _Arg2, typename _Arg3>
      ReaderThread(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3)
        : m_handler(arg1, arg2, arg3)
      {
      }

      /**
       * @param arg1 Passed tot the handler constructor.
       * @param arg2 Passed tot the handler constructor.
       * @param arg3 Passed tot the handler constructor.
       * @param arg4 Passed tot the handler constructor.
       */
      template<typename _Arg1, typename _Arg2, typename _Arg3, typename _Arg4>
      ReaderThread(_Arg1 arg1, _Arg2 arg2, _Arg3 arg3, _Arg4 arg4)
        : m_handler(arg1, arg2, arg3, arg4)
      {
      }


      ~ReaderThread() 
      {
        switch (m_stateManager.getState())
        {
          case READER_STATE_STARTING:
            waitForState(READER_STATE_RUNNING);
            stop();
            join();
            break;
          case READER_STATE_RUNNING:
            stop();
            join();
            break;
          case READER_STATE_STOPPING:
            join();
            break;
          default:
            break;
        }
      }
  };
}

