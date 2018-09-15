
#ifndef BASECOM_TIMEZONE_H
#define BASECOM_TIMEZONE_H

#include "copyable.h"

#include <memory>

#include <time.h>

//TimeZone for 1970~2030
class TimeZone : public copyable
{
 public:
  explicit TimeZone(const char* zonefile);
  TimeZone(int eastOfUtc, const char* tzname); //a fixed timezone  固定时区
  TimeZone() { } // an invalid timezone
  
  bool valid() const
  {
    return static_cast<bool>(data_);
  }
  
  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;
  
  //gmtime(3)
  static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
  
  //timegm(3)
  static time_t fromUtcTime(const struct tm&);
  
  //year in [1900..2500]
  static time_t fromUtcTime(int year, int month, int day,
                            int hour, int minute, int seconds);
  struct Data;
  
 private:
  std::shared_ptr<Data> data_;
};

#endif //BASECOM_TIMEZONE_H
