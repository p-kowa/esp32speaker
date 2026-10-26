#pragma once
#include <Arduino.h>

inline int32_t pcmPeak(const int16_t *samples, size_t count) {
    int32_t peak = 0;
    for (size_t i = 0; i < count; ++i) {
        peak = max(peak, abs(static_cast<int32_t>(samples[i])));
    }
    return peak;
}

// Peak normalization to 25000/32767 with at most 8x gain
inline float pcmGainForPeak(int32_t peak) {
    return peak > 0 ? min(8.0f, 25000.0f / peak) : 1.0f;
}
