#include <Rabotnik/ReaderThread.h>
#include <Rabotnik/StaticQueue.h>
#include <Rabotnik/PullBufferQueue.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace Rabotnik;
using namespace std;

typedef StaticQueue<unsigned int, 100> queue;
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
      std::cout << "Len " << q.length() << "\n";
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
          std::cout << "ZERO!\n";
        }

        i = i ^ (i << 1);
        ++i;
        i &= 0xFFFFFF;
      }
      usleep(10);
    }
};

typedef ReaderThread<PullBufferQueue<queue>, BufferHandler> reader_thread;

reader_thread g_readerThread;

void writer() 
{
  unsigned int d = 0;
  for(;;)
  {
    for (int i = 7; i <= 10; ++i)
    {
      for (int j = 0; j < i; ++j)
      {
        queue * q = &g_readerThread.beginWriting();
        while (q->length() > 90)
        {
          g_readerThread.finishWriting();
          usleep(10);
          q = &g_readerThread.beginWriting();
        }

        q->push_back(d);
        g_readerThread.finishWriting();
        d = d ^ (d << 1);
        ++d;
        d &= 0xFFFFFF;
      }
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

