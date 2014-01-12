#pragma once

#include <Rabotnik/ReaderState.h>
#include <boost/thread.hpp>

namespace Rabotnik 
{
  namespace Internal 
  {
    class StateManager 
    {
      ReaderState m_state;
      mutable boost::mutex m_stateMutex;
      mutable boost::condition_variable m_stateCond;

      public:
        StateManager()
          : m_state(READER_STATE_STOPPED)
        {
        }

        void setState(ReaderState state) {
          {
            boost::unique_lock<boost::mutex> lock(m_stateMutex);
            m_state = state;
          }
          m_stateCond.notify_one();
        }

        ReaderState getState() {
          boost::unique_lock<boost::mutex> lock(m_stateMutex); 
          return m_state;
        }

        void waitForState(ReaderState state) const
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
}

