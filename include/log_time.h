#pragma once
#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

// Time with milliseconds for log lines; milliseconds since startup before NTP synchronization.
inline String logTimestamp() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm info;
    char buf[24];
    if (localtime_r(&tv.tv_sec, &info) && info.tm_year > 120) {
        const size_t n = strftime(buf, sizeof(buf), "%H:%M:%S", &info);
        snprintf(buf + n, sizeof(buf) - n, ".%03d", (int)(tv.tv_usec / 1000));
    } else {
        snprintf(buf, sizeof(buf), "%lu ms", (unsigned long)millis());
    }
    return String(buf);
}
