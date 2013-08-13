#pragma once

#include <boost/type_traits/integral_constant.hpp>

/**
 * @brief Makes a template<_T, _Sign> class HasMemberFunction_functionName.
 *
 * The resulting class has static const bool value, true if the function exists
 * in _T, false otherwise.
 * Also, the class has typedef type, which is true_type if the function exists,
 * false_type otherwise.
 *
 * Usage:
 * \code
 *  HAS_MEMBER_FUNCTION(printHelloWorld);
 *  boost::enable_if<HasMemberFunction_printHelloWorld<HelloWorldPrinter, void ()>
 *
 * Code lifted from http://stackoverflow.com/questions/257288/is-it-possible-to-write-a-c-template-to-check-for-a-functions-existence/264088#264088
 */
#define HAS_MEMBER_FUNCTION(func) \
  template<typename _T, typename _Sign>\
  struct HasMemberFunction_##func\
  {\
    typedef char no[1];\
    typedef char yes[2];\
    template<typename _U, _U> struct type_check;\
    template<typename _1> static yes &chk(type_check<_Sign, &_1::func> *);\
    template<typename> static no &chk(...);\
    static const bool value = sizeof(chk<_T>(0)) == sizeof(yes);\
    typedef boost::integral_constant<bool, value> type;\
  }

