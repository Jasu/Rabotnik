Rabotnik
========

Simple, header-only producer-consumer threading library for making threads 
writing and reading to/from statically allocated buffers.

Dependencies
------------

  * Compile-time: Boost system, thread, concept\_check, type\_traits, 
    date\_time, utility, bind
  * Link-time: Boost system and thread.

Features
--------

  * Statically allocated producer-consumer buffer push queue (PushBufferQueue).
  * Statically allocated producer-consumer buffer pull queue (PullBufferQueue).
  * Statically allocated single-threaded queue (StaticQueue). Useful when used 
    as the buffer type in the producer-consumer buffer queue (BufferQueue).
  * Thread using producer-consumer queue. Calls a class method for each buffer,
    possibly with microseconds elapsed since last call. Also calls 
    initialization and uninitialization routines, if they exist. Can be used
    either with PushBufferQueue or PullBufferQueue.

Configuration
-------------

Define **RABOTNIK\_UNCHECKED** to disable bounds checking in StaticQueue.

