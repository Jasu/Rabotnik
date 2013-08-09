#pragma once

#include <Rabotnik/Exception.h>
#include <Rabotnik/Internal/BufferQueue.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/concept_check.hpp>

namespace Rabotnik
{
  /**
   * @brief Static-sized, multithreaded, single-consumer / single-producer 
   * buffer queue, where producer decides when the buffer is ready.
   *
   * @param _Buffer Type of the buffer object.
   * @param _BufferCount Number of buffers.
   */
  template<typename _Buffer, unsigned int _BufferCount>
  class PushBufferQueue
  {
    static const size_t m_maxBufferIndex = _BufferCount * sizeof(_Buffer);

    char m_buffers[_BufferCount * sizeof(_Buffer)];

    boost::condition_variable m_numFullBuffersCond;
    boost::mutex m_numFullBuffersMutex;

    unsigned int m_currentReadBuffer;
    unsigned int m_currentWriteBuffer;
    unsigned int m_numFullBuffers;

    public:
      typedef _Buffer buffer;

      PushBufferQueue()
        : m_currentReadBuffer(0),
          m_currentWriteBuffer(0),
          m_numFullBuffers(0)
      {
      }
      
      _Buffer & beginReading() 
      {
        if (m_numFullBuffers == 0)
        {
          boost::unique_lock<boost::mutex> lock(m_numFullBuffersMutex);
          while (m_numFullBuffers == 0)
          {
            m_numFullBuffersCond.wait(lock);
          }
        }
        return *(_Buffer *)&m_buffers[m_currentReadBuffer];
      }

      void finishReading()
      {
        ((_Buffer *)&m_buffers[m_currentReadBuffer])->~_Buffer();

        m_currentReadBuffer += sizeof(_Buffer);
        if (m_currentReadBuffer >= m_maxBufferIndex)
        {
          m_currentReadBuffer = 0;
        }

        {
          boost::unique_lock<boost::mutex> lock(m_numFullBuffersMutex);
          --m_numFullBuffers;
        }

        m_numFullBuffersCond.notify_one();
      }

      _Buffer & beginWriting() 
      {
        if (m_numFullBuffers == _BufferCount)
        {
          boost::unique_lock<boost::mutex> lock(m_numFullBuffersMutex);
          while (m_numFullBuffers == _BufferCount)
          {
            m_numFullBuffersCond.wait(lock);
          }
        }

        _Buffer * buffer 
          = reinterpret_cast<_Buffer*>(&m_buffers[m_currentWriteBuffer]);
        new (buffer) _Buffer();
        return *buffer;
      }

      void finishWriting()
      {
        m_currentWriteBuffer += sizeof(_Buffer);
        if (m_currentWriteBuffer >= m_maxBufferIndex)
        {
          m_currentWriteBuffer = 0;
        }

        {
          boost::unique_lock<boost::mutex> lock(m_numFullBuffersMutex);
          ++m_numFullBuffers;
        }

        m_numFullBuffersCond.notify_one();
      }

      ~PushBufferQueue()
      {
        for (int i = 0; i < m_numFullBuffers; ++i)
        {
          finishReading();
        }
      }

      BOOST_CONCEPT_ASSERT((Internal::BufferQueueConceptCheck<PushBufferQueue<_Buffer, _BufferCount> >));
  };
}
