#pragma once
#include <Arduino.h>

// ==========================================
// 1. I2S MAX98357A pin assignment for ESP32-S3
// ==========================================
#define I2S_BCLK        9   // Bit Clock (BCLK)
#define I2S_LRC         7   // Word Select / Left Right Clock (LRC / WS)
#define I2S_DOUT        8   // Data In on the MAX98357A (DIN)

// SPI microSD reader
#define SD_SCK          12
#define SD_MOSI         11
#define SD_MISO         13
#define SD_CS           10

// ==========================================
// 3. Audio settings
// ==========================================
#define DEFAULT_VOLUME          12  // 0 to 21 (default volume at startup)
#define DEFAULT_ANNOUNCE_VOLUME 14  // 0 to 21 (default volume for announcements / TTS)

// Stations are stored exclusively in M3U lists.

// ==========================================
// 4. I2S ICS43434 pin assignment for ESP32-S3 (microphone)
#define I2S_MIC_BCLK    14   // Bit Clock (BCLK)
#define I2S_MIC_LRC     16   // Word Select / Left Right Clock (LRC / WS)
#define I2S_MIC_DOUT    21   // Data Out from the ICS43434 (DOUT)
// ==========================================

// ==========================================
// 5. LED pin assignment for ESP32-S3
// ==========================================
#define LED_PIN         15   // Example: pin for a status LED
