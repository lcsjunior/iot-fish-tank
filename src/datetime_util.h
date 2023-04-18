#ifndef DATETIME_UTIL_H
#define DATETIME_UTIL_H

#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

#define MILLIS_PER_SECOND 1000UL

class DateTimeUtilClass {
public:
  time_t now() const;
  time_t uptimeSecs() const;
};

extern DateTimeUtilClass DateTimeUtil;

#endif // DATETIME_UTIL_H