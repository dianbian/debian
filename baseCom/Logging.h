
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
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };  
};

#endif //BASECOM_LOGGING_H