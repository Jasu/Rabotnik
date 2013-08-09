#pragma once

#include <boost/concept_check.hpp>

namespace Rabotnik
{
  namespace Internal
  {
    template <typename _BufferQueue>
    struct BufferQueueConceptCheck
    {
      BOOST_CONCEPT_USAGE(BufferQueueConceptCheck)
      {
        _BufferQueue q;
        typename _BufferQueue::buffer & bw = q.beginWriting();
        q.finishWriting();
        typename _BufferQueue::buffer & br = q.beginReading();
        q.finishReading();
      }
    };
  }
}
