#include "datetime_util.h"

time_t DateTimeUtilClass::now() const { return time(nullptr); }

time_t DateTimeUtilClass::uptime() const { return (time_t)(millis() / 1000); }

DateTimeUtilClass DateTimeUtil;