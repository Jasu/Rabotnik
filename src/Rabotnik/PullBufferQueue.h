#pragma once

#include <Rabotnik/Exception.h>
#include <Rabotnik/Internal/BufferQueue.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/concept_check.hpp>

namespace Rabotnik
{
  /**
   * @brief Static-sized, multithreaded, single-consumer / single-producer 
   * buffer queue, where consumer decides when the buffer is ready.
   *
   * Pull buffer queues with size != 2 don't make sense, hence number of 
   * buffers is not configurable.
   *
   * @param _Buffer Type of the buffer object.
   */
  template<typename _Buffer>
  class PullBufferQueue
  {
    char m_buffers[sizeof(_Buffer)*2];

    boost::recursive_mutex m_bufferMutex;

    unsigned int m_currentWriteBufferIndex;

    public:
      typedef _Buffer buffer;

      PullBufferQueue()
        : m_currentWriteBufferIndex(0)
      {
        new (&m_buffers[0]) _Buffer();
        new (&m_buffers[sizeof(_Buffer)]) _Buffer();
      }

      ~PullBufferQueue()
      {
        ((_Buffer&)m_buffers[0]).~_Buffer();
        ((_Buffer&)m_buffers[sizeof(_Buffer)]).~_Buffer();
      }
      
      /**
       * @brief Swaps buffers, returns the one that was just being written.
       */
      _Buffer & beginReading() 
      {
        boost::unique_lock<boost::recursive_mutex> lock(m_bufferMutex);
        m_currentWriteBufferIndex ^= sizeof(_Buffer);
        return *(_Buffer*)&m_buffers[m_currentWriteBufferIndex ^ sizeof(_Buffer)];
      }

      /**
       * @brief Recreates the buffer the application is done with.
       */
      void finishReading()
      {
        ((_Buffer&)m_buffers[m_currentWriteBufferIndex ^ sizeof(_Buffer)]).~_Buffer();
        new (&m_buffers[m_currentWriteBufferIndex ^ sizeof(_Buffer)]) _Buffer();
      }

      _Buffer & beginWriting() 
      {
        m_bufferMutex.lock();
        return *(_Buffer*)&m_buffers[m_currentWriteBufferIndex];
      }

      void finishWriting()
      {
        m_bufferMutex.unlock();
      }

      BOOST_CONCEPT_ASSERT((Internal::BufferQueueConceptCheck<PullBufferQueue<_Buffer> >));
  };
}

