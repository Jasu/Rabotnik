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
   * @brief Wrapper for a callback which reads a buffer.
   *
   * @param _BufferHandler
   *  Type to handle the events. Contained within the ReaderThread.
   *  Must have void processBuffer() or void processBuffer(
   *  unsigned int microsecondsFromLastCall)
   *  May have void initializeThread() and/or void uninitializeThread().
   *
   * @param _CallbackManagerPtr 
   *  Pointer to an object having startCallback() and stopCallbac() functions.
   *  The pointer is not deleted by CallbackReader and may be any pointer-like
   *  object, eg. shared_ptr<>. Since it is not deleted, it may be a pointer
   *  to the object owning the CallbackReader.
   */
  template<
    typename _BufferQueue, 
    typename _BufferHandler,
    typename _CallbackManagerPtr
  >
  class CallbackReader
  {
    typedef typename _BufferQueue::buffer buffer;
    boost::thread m_thread;
    bool m_isCallbackRunning;

    _BufferQueue m_bufferQueue;

    _BufferHandler m_handler;

    _CallbackManagerPtr m_callbackManager;

    Internal::ProcessBufferCaller<_BufferHandler, buffer> m_processBufferCaller;

    Internal::StateManager m_stateManager;

    public:
      CallbackReader(_CallbackManagerPtr callbackManager) 
        : m_callbackManager(callbackManager)
      {
      }

      buffer & beginWriting() { return m_bufferQueue.beginWriting(); }
      void finishWriting() { m_bufferQueue.finishWriting(); }

      _BufferHandler & getBufferHandler() { return m_handler; }
      const _BufferHandler & getBufferHandler() const { return m_handler; }
      
      void callback() 
      {
        switch (m_stateManager.getState())
        {
          case READER_STATE_STARTING:
            Internal::callInitializeThread(m_handler);
            m_stateManager.setState(READER_STATE_RUNNING);
            //Fall-through to running.
          case READER_STATE_RUNNING:
            {
              buffer & buffer = m_bufferQueue.beginReading();
              m_processBufferCaller.call(m_handler, buffer);
              m_bufferQueue.finishReading();
            }
            break;
          case READER_STATE_STOPPING:
            Internal::callUninitializeThread(m_handler);
            m_callbackManager->stopCallback();
            m_stateManager.setState(READER_STATE_STOPPED);
            break;
          case READER_STATE_STOPPED:
            break;
        }
      }

      /**
       * @brief Only notifies that the thread is starting. The user
       *  responsible for actually starting the callback.
       */
      void start()
      {
        m_stateManager.setState(READER_STATE_STARTING);
        m_callbackManager->startCallback();
      }

      /**
       * @brief Only notifies that the thread is stopping. The user
       *  responsible for actually stopping the callback.
       */
      void stop()
      {
        m_stateManager.setState(READER_STATE_STOPPING);
      }

      void join()
      {
        m_stateManager.waitForState(READER_STATE_STOPPED);
      }

      void waitForState(ReaderState state) const
      {
        m_stateManager.waitForState(state);
      }

      ~CallbackReader()
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

