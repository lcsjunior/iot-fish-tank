#include "datetime_util.h"

time_t epoch;
static const char *dateTimeFormat = "%A, %B %d %Y %H:%M:%S";
static const char *millisFormat = "%02d:%02d:%02d.%03d";

time_t *now() {
  time(&epoch);
  return &epoch;
}

char *cdttime(time_t *t) {
  struct tm *timeinfo;
  timeinfo = localtime(t);
  size_t len = 64;
  char *buf = (char *)malloc(len);
  strftime(buf, len, dateTimeFormat, timeinfo);
  return buf;
}

char *cmillis() {
  unsigned long durationInMillis = millis();
  int ms = durationInMillis % 1000;
  int sec = (durationInMillis / 1000) % 60;
  int min = (durationInMillis / (1000 * 60)) % 60;
  int hr = (durationInMillis / (1000 * 60 * 60)) /* % 24*/;
  char *buf = (char *)malloc(32);
  sprintf_P(buf, millisFormat, hr, min, sec, ms);
  return buf;
}

void printReadableLocalTime() { Serial.println(FPSTR(cdttime(now()))); }

void printReadableMillis() { Serial.println(FPSTR(cmillis())); }
