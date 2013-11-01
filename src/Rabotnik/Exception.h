#pragma once

#include <stdexcept>

namespace Rabotnik
{
  class Exception : public std::exception
  {
    const char * m_what;

    public:
      Exception(const char * what)
        : m_what(what)
      {
      }

      const char * what() const throw()
      {
        return m_what;
      }
  };
}
