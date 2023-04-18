#include "datetime_util.h"

time_t DateTimeUtilClass::now() const { return time(nullptr); }

time_t DateTimeUtilClass::uptimeSecs() const {
  return (time_t)(millis() / 1000);
}

char *DateTimeUtilClass::format(char *buf, const char *fmt) const {
  size_t len = 64;
  time_t t = now();
  struct tm *timeinfo = localtime(&t);
  strftime(buf, len, fmt, timeinfo);
  return buf;
}

char *DateTimeUtilClass::formatUTC(time_t *t, char *buf,
                                   const char *fmt) const {
  size_t len = 64;
  struct tm *timeinfo = gmtime(t);
  strftime(buf, len, fmt, timeinfo);
  return buf;
}

DateTimeUtilClass DateTimeUtil;