#pragma once
#include <Arduino.h>
#include <sys/time.h>
#include <time.h>

// Uhrzeit mit Millisekunden fuer Log-Zeilen; vor dem NTP-Abgleich Millisekunden seit Start.
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
