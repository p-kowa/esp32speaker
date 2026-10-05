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
#define DEFAULT_VOLUME          12  // 0 bis 21 (Standardlautstärke beim Start)
#define DEFAULT_ANNOUNCE_VOLUME 14  // 0 bis 21 (Standardlautstärke für Durchsagen / TTS)

// Sender werden ausschließlich über M3U-Listen gespeichert.

// ==========================================
// 4. I2S I²S ICS43434 Pin-Belegung für ESP32-S3 (Mikrofon)
#define I2S_MIC_BCLK    14   // Bit Clock (BCLK)
#define I2S_MIC_LRC     16   // Word Select / Left Right Clock (LRC / WS)
#define I2S_MIC_DOUT    21   // Data Out vom ICS43434 (DOUT)
// ==========================================