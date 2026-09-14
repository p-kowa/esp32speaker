#pragma once
#include <Arduino.h>

// ==========================================
// 1. I2S MAX98357A Pin-Belegung für ESP32-S3
// ==========================================
#define I2S_BCLK        9   // Bit Clock (BCLK)
#define I2S_LRC         7   // Word Select / Left Right Clock (LRC / WS)
#define I2S_DOUT        8   // Data In am MAX98357A (DIN)

// SPI Micro-SD Leser
#define SD_SCK          12
#define SD_MOSI         11
#define SD_MISO         13
#define SD_CS           10

// ==========================================
// 3. Audio-Einstellungen
// ==========================================
#define DEFAULT_VOLUME  12  // 0 bis 21 (Standardlautstärke beim Start)

// Sender werden ausschließlich über M3U-Listen gespeichert.
