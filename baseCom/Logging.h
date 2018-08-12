
#ifndef BASECOM_LOGGING_H
#define BASECOM_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

class TimeZone;

class Logger
{
 public:
  enum LogLevel
  {
    TRACE,
    DEBUG1,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  //compile time calculation of basename of source file
  class SourceFile
  {
   public:
    template<int N>
    inline SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1)
    {
      const char* slash = strrchr(data_, '/');
      if (slash)
      {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }
    
    explicit SourceFile(const char* filename) : data_(filename)
    {
      const char* slash = strrchr(filename, '/');
      if (slash)
      {
        data_ = slash + 1;
      }
      size_ -= static_cast<int>(strlen(data_));
    }
    
    const char* data_;
    int size_;
  };
  
  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbotr);
  ~Logger();
  
  LogStream& stream() { return impl_.stream_; }
  
  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);
  
  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setTimeZone(const TimeZone& tz);

 private:
  class Impl
  {
   public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void formatTime();
    void finish();
    
    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };
  
  Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
        Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
        Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()    
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
        Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

#define CHECK_NOTNULL(val) \
        CheckNotNULL(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val));
        
template <typename T>
T* CheckNotNULL(Logger::SourceFile file, int line, const char* names, T* ptr)
{
  if (ptr == NULL)
  {
    Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

#endif //BASECOM_LOGGING_H
