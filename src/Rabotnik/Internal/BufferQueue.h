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
        void dummy_to_prevent_warnings(typename _BufferQueue::buffer & bw);
        _BufferQueue q;
        dummy_to_prevent_warnings(q.beginWriting());
        q.finishWriting();
        dummy_to_prevent_warnings(q.beginReading());
        q.finishReading();
      }
    };
  }
}
