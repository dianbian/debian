
#ifndef BASECOM_TYPES_H
#define BASECOM_TYPES_H

#include <stdint.h>
#ifndef NDEBUG
#include <assert.h>
#endif

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
  return f;
}

template<typename To, typename From>
inline To down_cast(From* f)
{
  if (false)
  {
    implicit_cast<From*, To>(0);
  }
  
#if !define(NDEBUG) && !define(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == NULL ||dynamic_cast<To>(f) != NULL);
#endif
  return static_cast<To>(f);
}

#endif
