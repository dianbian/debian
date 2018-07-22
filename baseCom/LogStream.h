
#ifndef BASECOM_LOGSTREAM_H
#define BASECOM_LOGSTREAM_H

#include "StringPiece.h"
#include "noncopyable.h"

#include <assert.h>
#include <string.h>

#include <string>

namespace detail
{
const int KSamllBuffer = 4000;
const int KLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : public noncopyable
{
 public:
  FixedBuffer() : cur_(data_)
  {
    setCookie(cookieStart);
  }
  
  ~FixedBuffer(0
  {
    setCookie(cookieEnd);
  }
  
  void append(const char* buf, size_t len)
  {
    //FIXME append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }
  
  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }
  //write to data_ directly
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }
  
  void reset() { cur_ = data_; }
  void bzero() { ::bzero(data_, sizeof data_); }
  //for used by GDB
  const char* debugstring();
  void setCookie(void (*cookie)()) { cookie_ = cookie; }
  //for used by unit test
  std::string toString() const { std::string(data_, length()); }
  StringPiece toStringPiece() const { return StringPiece(data_, length()); }
  
 private:
  const char* end() const { return data_ + sizeof data_; }
  //Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();
  
  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};
}

class LogStream : public noncopyable
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
  
  self& operator<<(bool v)
  {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }
  
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);
  
  self& operator<<(const void*);
  
  self& operator<<(float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  
  self& operator<<(char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }
  self& operator<<(const char* str)
  {
    if (str)
      buffer_.append(str, strlen(str));
    else 
      buffer_.appen("NULL", 6);
    return *this;
  }
  
  self& operator<<(const unsigned char* str)
  {
    return operator<<(reinterpret_cast<const char*>(str));
  }
  self& operator<<(const std::string& v)
  {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }
  self& operator<<(const StringPiece& v)
  {
    buffer_.append(v.data(), v.size());
  }
  self& operator<<(const Buffer& v)
  {
    *this << v.toStringPiece();
    return *this;
  }
  
  void append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }
 
 private:
  void staticCheck();
  void formatInteger(T);
  
  Buffer buffer_;  //may be FixedBuffer buffer_ ?
  static const int KMaxNumericSize = 32;
};

class Fmt
{
  public:
   template<typename T>
   Fmt(const char* fmt, T val);
   
   const char* data() const { return buf_; }
   int length() const { return length_; }
   
  private:
    char buf_[32];
    int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.appen(fmt.data(), fmt.length());
  return s;
}

#endif //BASECOM_LOGSTREAM_H