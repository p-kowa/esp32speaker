#pragma once
#include <Arduino.h>
#include <driver/i2s.h> 
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>

class MicManager {
public:
    static constexpr uint32_t SAMPLE_RATE = 16000;
    static constexpr size_t FRAME_SAMPLES = 512;

    esp_err_t begin(int bclk, int ws, int din);
    void end();
    bool isRunning() const { return task_ != nullptr; }
    float levelDbfs() const;
    esp_err_t lastError() const { return lastError_; }
    void setStreamingEnabled(bool on);
    size_t read(int16_t *dst, size_t samples, TickType_t timeout);

private:
    static void taskEntry(void *arg);
    void run();

    static constexpr i2s_port_t PORT = I2S_NUM_1;
    TaskHandle_t task_ = nullptr;
    StreamBufferHandle_t pcm_ = nullptr;
    volatile bool stop_ = false;
    volatile bool streaming_ = false;
    mutable portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    float dbfs_ = -60.0f;
    esp_err_t lastError_ = ESP_OK;
    int32_t raw_[FRAME_SAMPLES];
    int16_t pcmFrame_[FRAME_SAMPLES];
};