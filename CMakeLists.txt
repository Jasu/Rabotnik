cmake_minimum_required(VERSION 2.8.6)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0004 NEW)


if (NOT DEFINED NO_FIND_BOOST) 
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)

  find_package(Boost COMPONENTS system thread)
endif ()

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)

