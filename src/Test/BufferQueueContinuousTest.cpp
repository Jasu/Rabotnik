#include <Rabotnik/ReaderThread.h>
#include <Rabotnik/StaticQueue.h>
#include <Rabotnik/PushBufferQueue.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

//Hack to get around getpagesize() working on iOS SDK where it is deprecated
//and removed when _POSIX_SOURCE is defined. It is required by thread.hpp.
extern "C" { int getpagesize(); }
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace Rabotnik;
using namespace std;

typedef StaticQueue<unsigned int, 10> queue;
//typedef BufferQueue<queue, 2> message_buffer;
//message_buffer g_buffer;


class BufferHandler
{
  unsigned int i;

  public:
    BufferHandler()
      : i(0)
    {
    }

    void processBuffer(queue & q, unsigned int usec) 
    {
      unsigned int c = 0;
      BOOST_FOREACH(unsigned int u, q)
      {
        if (u != i)
        {
          std::cerr << "FAILURE!" << std::endl;
          exit(0);
        }
        if (u == 0)
        {
          std::cerr << "ZERO!" << std::endl;
        }

        i = i ^ (i << 1);
        ++i;
        i &= 0xFFFFFF;
      }
    }
};

typedef ReaderThread<PushBufferQueue<queue, 3>, BufferHandler> reader_thread;

reader_thread g_readerThread;

void writer() 
{
  unsigned int d = 0;
  for(;;)
  {
    for (int i = 7; i <= 10; ++i)
    {
      queue & q = g_readerThread.beginWriting();
      q.clear();
      queue::writer w = q.beginWriting();
      for (int j = 0; j < i; ++j)
      {
        w.push_back(d);
        d = d ^ (d << 1);
        ++d;
        d &= 0xFFFFFF;
      }
      q.finishWriting(w);
      g_readerThread.finishWriting();
    }
  }
}

int main() 
{
  boost::thread w(writer);
  g_readerThread.start();
  w.join();
  g_readerThread.stop();
  g_readerThread.join();
}

