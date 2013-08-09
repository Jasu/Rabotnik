#pragma once

#include <Rabotnik/Exception.h>

#include <boost/utility.hpp>
#include <iostream>

namespace Rabotnik
{

  /**
   * @brief Small writer for writing multiple object into the queue fast.
   * 
   * Removes one pointer indirection and n-1 write pointer updates when writing
   * n values using the writer.
   */
  template<typename _T>
  class StaticQueueWriter
  {
    mutable _T * m_writePointer;
#ifndef RABOTNIK_UNCHECKED
    size_t m_sizeLeft;
#endif
    public:
#ifndef RABOTNIK_UNCHECKED
      StaticQueueWriter(_T * firstWritePointer, size_t sizeLeft)
        : m_writePointer(firstWritePointer),
          m_sizeLeft(sizeLeft)
#else
      StaticQueueWriter(_T * firstWritePointer)
        : m_writePointer(firstWritePointer)
#endif
      {
      }

      _T & push_back() 
      {
#ifndef RABOTNIK_UNCHECKED
        if (!m_sizeLeft--)
        {
          throw Exception("Overflow in _T & StaticQueueWriter::push_back().");
        }
#endif
        
        return *m_writePointer++;
      }
      
      void push_back(const _T & value)
      {
#ifndef RABOTNIK_UNCHECKED
        if (!m_sizeLeft--)
        {
          throw Exception("Overflow in _T & StaticQueueWriter::push_back().");
        }
#endif
        *m_writePointer++ = value;
      }

      _T * const getWritePointer() const { return m_writePointer; }
  };

  /**
   * @brief Statically allocated queue.
   *
   * Allocates the message buffer using the default constructor.
   * Popping does not call destructor. That is, this class is usually suitable
   * only for storing pure data objects.
   */
  template<typename _T, size_t _NumItems>
  class StaticQueue : boost::noncopyable
  {
    _T m_queue[_NumItems];
    _T * m_writePointer;

    public:
      typedef _T * iterator;
      typedef const _T * const_iterator;

      StaticQueue()
        : m_writePointer(&m_queue[0])
      {
      }

      const_iterator begin() const
      {
        return &m_queue[0];
      }

      iterator begin()
      {
        return &m_queue[0];
      }

      const_iterator end() const
      {
        return m_writePointer;
      }

      iterator end()
      {
        return m_writePointer;
      }

      /**
       * @brief Invalidates iterators.
       */
      void push_back(const _T & item)
      {
#ifndef RABOTNIK_UNCHECKED
        if (m_writePointer >= &m_queue[_NumItems])
        {
          throw Exception("Overflow in void StaticQueue::push_back(const _T &).");
        }
#endif
        *m_writePointer++ = item;
      }

      /**
       * @brief Invalidates iterators. Marks the item returned as complete, so
       * if you are using this, you cannot be reading at the same time.
       */
      _T & push_back()
      {
#ifndef RABOTNIK_UNCHECKED
        if (m_writePointer >= &m_queue[_NumItems])
        {
          throw Exception("Overflow in _T & StaticQueue::push_back().");
        }
#endif
        return *m_writePointer++;
      }

      size_t length() const
      {
        return m_writePointer - &m_queue[0];
      }

      void clear() {
        m_writePointer = &m_queue[0];
      }

      /**
       * @addtogroup Writing with a writer
       * @{
       */

      typedef StaticQueueWriter<_T> writer;

      writer beginWriting()
      {
#ifndef RABOTNIK_UNCHECKED
        return writer(m_writePointer, _NumItems - length());
#else
        return writer(m_writePointer);
#endif
      }

      void finishWriting(const writer w)
      {
        m_writePointer = w.getWritePointer();
      }

      /** @} */
  };
}

