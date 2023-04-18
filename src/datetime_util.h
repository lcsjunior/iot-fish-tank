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
  char *format(char *buf, const char *fmt) const;
  char *formatUTC(time_t *t, char *buf, const char *fmt) const;
};

struct DateFormatter {
  constexpr static const char *HTTP = "%a, %d %b %Y %H:%M:%S GMT";
  constexpr static const char *SIMPLE = "%Y:%m:%d %H:%M:%S";
  constexpr static const char *COMPAT = "%Y%m%d_%H%M%S";
  constexpr static const char *DATE_ONLY = "%F";
  constexpr static const char *TIME_ONLY = "%T";
};

extern DateTimeUtilClass DateTimeUtil;

#endif // DATETIME_UTIL_H