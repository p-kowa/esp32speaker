#pragma once
#include <stddef.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class MicManager;

// microWakeWord-Erkennung (TFLite Micro) in eigenem Task, liest PCM ueber MicManager
class WakeWordDetector {
public:
    struct Settings {
        float probabilityCutoff = 0.9f;
        uint8_t slidingWindowSize = 5;
        size_t tensorArenaSize = 32000;
        uint8_t featureStepMs = 10;
        uint16_t endOfSpeechSilenceMs = 1000;
    };

    // model: 16-Byte-ausgerichteter Puffer (heap_caps), geht in den Besitz des Detektors ueber
    bool begin(MicManager *mic, uint8_t *model, const Settings &settings);
    void end();

    bool isRunning() const { return task_ != nullptr; }
    const char *error() const { return error_; }
    bool takeDetection();
    void startRecording();
    bool recordingComplete() const { return recordingComplete_; }
    bool recordingHasSpeech() const { return recordingHasSpeech_; }
    const int16_t *recordingData() const;
    size_t recordingSampleCount() const;
    void releaseRecording();
    float takePeakProbability();
    uint32_t detectionCount() const { return detections_; }

private:
    struct Impl;

    static void taskEntry(void *arg);
    void run();
    bool setup();
    void release();
    bool processFeatures(const int8_t *features);

    MicManager *mic_ = nullptr;
    uint8_t *model_ = nullptr;
    Settings settings_;
    Impl *impl_ = nullptr;
    TaskHandle_t task_ = nullptr;
    volatile bool stop_ = false;
    volatile bool detected_ = false;
    volatile bool recordingRequested_ = false;
    volatile bool recordingComplete_ = false;
    volatile bool recordingHasSpeech_ = false;
    volatile bool recordingReleaseRequested_ = false;
    volatile uint32_t detections_ = 0;
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    float peak_ = 0.0f;
    const char *error_ = "";
};

extern WakeWordDetector wakeDetector;
