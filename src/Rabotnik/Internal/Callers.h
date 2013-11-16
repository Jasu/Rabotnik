#pragma once
/**
 * @file
 * Contains utilities to call functions in BufferHandler.
 */

#include <Rabotnik/Internal/Utilities.h>

#include <boost/utility.hpp>
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
}

