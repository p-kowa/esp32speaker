#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

// 16-Bit-PCM, mono
void makeWavHeader(uint8_t header[44], uint32_t sampleCount, uint32_t sampleRate);

// Erwartet ein Array aus {"freq": Hz, "duration": ms, "type": "sine|square|triangle|sawtooth"}; freq 0 = Pause
bool writeToneWav(fs::FS &filesystem, const char *path, JsonArrayConst tones, String &error);
