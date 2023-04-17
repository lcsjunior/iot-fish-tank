#ifndef DATETIME_UTIL_H
#define DATETIME_UTIL_H

#include <Arduino.h>
#include <sys/time.h>

time_t *now();
char *cdttime(time_t *t);
char *cmillis();
void printReadableLocalTime();
void printReadableMillis();

#endif // DATETIME_UTIL_H