#include "mic_manager.h"
#include <math.h>

void MicManager::end() {
    if (!task_) return;
    stop_ = true;
    while (task_) vTaskDelay(pdMS_TO_TICKS(10));
}

void MicManager::taskEntry(void *arg) {
    static_cast<MicManager *>(arg)->run();
}

void MicManager::run() {
    float smoothed = -60.0f;
    while (!stop_) {
        size_t bytes = 0;
        if (i2s_read(PORT, raw_, sizeof(raw_), &bytes, pdMS_TO_TICKS(100)) != ESP_OK || bytes == 0) continue;

        const size_t n = bytes / sizeof(int32_t);
        double sum = 0.0;
        for (size_t i = 0; i < n; ++i) {
            // 24 Bit MSB-ausgerichtet -> int16; Verstärkung über Shift (14 = +12 dB) anpassbar
            const int16_t s = static_cast<int16_t>(raw_[i] >> 16);
            pcmFrame_[i] = s;
            const double v = s / 32768.0;
            sum += v * v;
        }

        const double rms = sqrt(sum / n);
        float db = rms > 0.0 ? 20.0f * log10f(static_cast<float>(rms)) : -60.0f;
        db = constrain(db, -60.0f, 0.0f);
        smoothed = smoothed * 0.7f + db * 0.3f;
        portENTER_CRITICAL(&mux_);
        dbfs_ = smoothed;
        portEXIT_CRITICAL(&mux_);

        if (streaming_) xStreamBufferSend(pcm_, pcmFrame_, n * sizeof(int16_t), 0);
    }

    i2s_stop(PORT);
    i2s_driver_uninstall(PORT);
    vStreamBufferDelete(pcm_);
    pcm_ = nullptr;
    portENTER_CRITICAL(&mux_);
    dbfs_ = -60.0f;
    portEXIT_CRITICAL(&mux_);
    task_ = nullptr;
    vTaskDelete(nullptr);
}

float MicManager::levelDbfs() const {
    portENTER_CRITICAL(&mux_);
    const float v = dbfs_;
    portEXIT_CRITICAL(&mux_);
    return v;
}

void MicManager::setStreamingEnabled(bool on) {
    if (on && pcm_) xStreamBufferReset(pcm_);
    streaming_ = on;
}

size_t MicManager::read(int16_t *dst, size_t samples, TickType_t timeout) {
    if (!pcm_) return 0;
    return xStreamBufferReceive(pcm_, dst, samples * sizeof(int16_t), timeout) / sizeof(int16_t);
}

esp_err_t MicManager::begin(int bclk, int ws, int din) {
    if (task_) return ESP_OK;

    i2s_config_t cfg = {};
    cfg.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX);
    cfg.sample_rate = SAMPLE_RATE;
    cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
    // ICS43434 L/R-Pin auf GND -> linker Slot
    cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    cfg.dma_buf_count = 6;
    cfg.dma_buf_len = 256;

    esp_err_t err = i2s_driver_install(PORT, &cfg, 0, nullptr);
    if (err != ESP_OK) return lastError_ = err;

    i2s_pin_config_t pins = {};
    pins.mck_io_num = I2S_PIN_NO_CHANGE;
    pins.bck_io_num = bclk;
    pins.ws_io_num = ws;
    pins.data_out_num = I2S_PIN_NO_CHANGE;
    pins.data_in_num = din;
    err = i2s_set_pin(PORT, &pins);
    if (err != ESP_OK) {
        i2s_driver_uninstall(PORT);
        return lastError_ = err;
    }

    pcm_ = xStreamBufferCreate(SAMPLE_RATE * sizeof(int16_t), FRAME_SAMPLES * sizeof(int16_t));
    stop_ = false;
    if (!pcm_ || xTaskCreatePinnedToCore(taskEntry, "mic", 4096, this, 5, &task_, 0) != pdPASS) {
        if (pcm_) vStreamBufferDelete(pcm_);
        pcm_ = nullptr;
        task_ = nullptr;
        i2s_driver_uninstall(PORT);
        return lastError_ = ESP_ERR_NO_MEM;
    }
    lastError_ = ESP_OK;
    return ESP_OK;
}