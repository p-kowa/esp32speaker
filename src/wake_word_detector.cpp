// TFLM-Header vor Arduino.h einbinden, damit Arduino-Makros sie nicht stoeren
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_resource_variable.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <frontend.h>
#include <frontend_util.h>

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <new>
#include "wake_word_detector.h"
#include "mic_manager.h"

namespace {
constexpr size_t FEATURE_SIZE = 40;
constexpr size_t FEATURE_WINDOW_MS = 30;
constexpr size_t SAMPLES_PER_MS = 16;
constexpr size_t VARIABLE_ARENA_SIZE = 1024;
constexpr size_t MAX_SLIDING_WINDOW = 32;
constexpr size_t MAX_RECORDING_SECONDS = 10;
constexpr size_t MAX_RECORDING_SAMPLES = MicManager::SAMPLE_RATE * MAX_RECORDING_SECONDS;
constexpr float SPEECH_GATE_DBFS = -45.0f;
// Reaktionszeit nach dem Wake Word, bevor die Stille-Zeit ohne Sprache zaehlt
constexpr uint32_t NO_SPEECH_GRACE_MS = 1000;
// Nach Start/Erkennung ~0,74 s ignorieren, bis der Modellzustand eingeschwungen ist (wie ESPHome)
constexpr int MIN_SLICES_BEFORE_DETECTION = 74;
constexpr size_t TASK_STACK = 10240;
}

struct WakeWordDetector::Impl {
    tflite::MicroMutableOpResolver<20> resolver;
    tflite::MicroInterpreter *interpreter = nullptr;
    tflite::MicroAllocator *varAllocator = nullptr;
    tflite::MicroResourceVariables *variables = nullptr;
    uint8_t *arena = nullptr;
    uint8_t *varArena = nullptr;
    FrontendConfig frontendConfig = {};
    FrontendState frontendState = {};
    bool frontendReady = false;
    int16_t samples[160 * 4] = {};
    size_t stride = 1;
    size_t strideStep = 0;
    uint8_t probabilities[MAX_SLIDING_WINDOW] = {};
    size_t probabilityIndex = 0;
    int ignoreSlices = -MIN_SLICES_BEFORE_DETECTION;
    int16_t recording[MAX_RECORDING_SAMPLES] = {};
    size_t recordingSamples = 0;
    uint32_t recordingStartedMs = 0;
    uint32_t silenceStartedMs = 0;
    bool recordingActive = false;
    bool speechSeen = false;
};

static void *allocPsram(size_t size) {
    void *ptr = heap_caps_aligned_alloc(16, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!ptr) ptr = heap_caps_aligned_alloc(16, size, MALLOC_CAP_8BIT);
    return ptr;
}

bool WakeWordDetector::begin(MicManager *mic, uint8_t *model, const Settings &settings) {
    end();
    mic_ = mic;
    model_ = model;
    settings_ = settings;
    if (settings_.slidingWindowSize < 1) settings_.slidingWindowSize = 1;
    if (settings_.slidingWindowSize > MAX_SLIDING_WINDOW) settings_.slidingWindowSize = MAX_SLIDING_WINDOW;
    if (settings_.featureStepMs < 1 || settings_.featureStepMs > 40) settings_.featureStepMs = 10;
    detected_ = false;
    recordingRequested_ = false;
    recordingComplete_ = false;
    recordingReleaseRequested_ = false;
    detections_ = 0;
    peak_ = 0.0f;
    error_ = "";

    if (!mic_ || !model_ || !setup()) {
        Serial.printf("[WAKE] Start fehlgeschlagen: %s\n", error_);
        release();
        return false;
    }

    mic_->setStreamingEnabled(true);
    stop_ = false;
    if (xTaskCreatePinnedToCore(taskEntry, "wakeword", TASK_STACK, this, 4, &task_, 0) != pdPASS) {
        task_ = nullptr;
        error_ = "Task konnte nicht gestartet werden";
        mic_->setStreamingEnabled(false);
        release();
        return false;
    }
    Serial.printf("[WAKE] Erkennung gestartet (Schwelle %.2f, Fenster %u, Stride %u)\n",
                  settings_.probabilityCutoff, settings_.slidingWindowSize, (unsigned)impl_->stride);
    return true;
}

void WakeWordDetector::end() {
    if (task_) {
        stop_ = true;
        while (task_) vTaskDelay(pdMS_TO_TICKS(10));
        Serial.println("[WAKE] Erkennung gestoppt.");
    }
    if (mic_) mic_->setStreamingEnabled(false);
    release();
}

bool WakeWordDetector::setup() {
    void *mem = heap_caps_malloc(sizeof(Impl), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!mem) mem = heap_caps_malloc(sizeof(Impl), MALLOC_CAP_8BIT);
    if (!mem) { error_ = "Kein Speicher"; return false; }
    impl_ = new (mem) Impl();
    Impl &d = *impl_;

    d.frontendConfig.window.size_ms = FEATURE_WINDOW_MS;
    d.frontendConfig.window.step_size_ms = settings_.featureStepMs;
    d.frontendConfig.filterbank.num_channels = FEATURE_SIZE;
    d.frontendConfig.filterbank.lower_band_limit = 125.0f;
    d.frontendConfig.filterbank.upper_band_limit = 7500.0f;
    d.frontendConfig.noise_reduction.smoothing_bits = 10;
    d.frontendConfig.noise_reduction.even_smoothing = 0.025f;
    d.frontendConfig.noise_reduction.odd_smoothing = 0.06f;
    d.frontendConfig.noise_reduction.min_signal_remaining = 0.05f;
    d.frontendConfig.pcan_gain_control.enable_pcan = 1;
    d.frontendConfig.pcan_gain_control.strength = 0.95f;
    d.frontendConfig.pcan_gain_control.offset = 80.0f;
    d.frontendConfig.pcan_gain_control.gain_bits = 21;
    d.frontendConfig.log_scale.enable_log = 1;
    d.frontendConfig.log_scale.scale_shift = 6;
    if (!FrontendPopulateState(&d.frontendConfig, &d.frontendState, MicManager::SAMPLE_RATE)) {
        error_ = "Audio-Frontend konnte nicht initialisiert werden";
        return false;
    }
    d.frontendReady = true;

    if (d.resolver.AddCallOnce() != kTfLiteOk || d.resolver.AddVarHandle() != kTfLiteOk ||
        d.resolver.AddReshape() != kTfLiteOk || d.resolver.AddReadVariable() != kTfLiteOk ||
        d.resolver.AddStridedSlice() != kTfLiteOk || d.resolver.AddConcatenation() != kTfLiteOk ||
        d.resolver.AddAssignVariable() != kTfLiteOk || d.resolver.AddConv2D() != kTfLiteOk ||
        d.resolver.AddMul() != kTfLiteOk || d.resolver.AddAdd() != kTfLiteOk ||
        d.resolver.AddMean() != kTfLiteOk || d.resolver.AddFullyConnected() != kTfLiteOk ||
        d.resolver.AddLogistic() != kTfLiteOk || d.resolver.AddQuantize() != kTfLiteOk ||
        d.resolver.AddDepthwiseConv2D() != kTfLiteOk || d.resolver.AddAveragePool2D() != kTfLiteOk ||
        d.resolver.AddMaxPool2D() != kTfLiteOk || d.resolver.AddPad() != kTfLiteOk ||
        d.resolver.AddPack() != kTfLiteOk || d.resolver.AddSplitV() != kTfLiteOk) {
        error_ = "TFLM-Operationen konnten nicht registriert werden";
        return false;
    }

    const tflite::Model *model = tflite::GetModel(model_);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_ = "Modell-Schema wird nicht unterstuetzt";
        return false;
    }

    d.arena = static_cast<uint8_t *>(allocPsram(settings_.tensorArenaSize));
    d.varArena = static_cast<uint8_t *>(allocPsram(VARIABLE_ARENA_SIZE));
    if (!d.arena || !d.varArena) {
        error_ = "Kein Speicher fuer Tensor-Arena";
        return false;
    }
    d.varAllocator = tflite::MicroAllocator::Create(d.varArena, VARIABLE_ARENA_SIZE);
    d.variables = d.varAllocator ? tflite::MicroResourceVariables::Create(d.varAllocator, 20) : nullptr;
    if (!d.variables) {
        error_ = "Ressourcen-Variablen konnten nicht angelegt werden";
        return false;
    }

    void *interpreterMem = heap_caps_malloc(sizeof(tflite::MicroInterpreter), MALLOC_CAP_8BIT);
    if (!interpreterMem) { error_ = "Kein Speicher fuer Interpreter"; return false; }
    d.interpreter = new (interpreterMem) tflite::MicroInterpreter(model, d.resolver, d.arena, settings_.tensorArenaSize, d.variables);
    if (d.interpreter->AllocateTensors() != kTfLiteOk) {
        error_ = "Tensoren konnten nicht angelegt werden (tensor_arena_size zu klein?)";
        return false;
    }

    TfLiteTensor *input = d.interpreter->input(0);
    if (input->dims->size != 3 || input->dims->data[0] != 1 || input->dims->data[2] != (int)FEATURE_SIZE ||
        input->type != kTfLiteInt8) {
        error_ = "Modell-Eingang passt nicht (erwartet int8 [1, n, 40])";
        return false;
    }
    TfLiteTensor *output = d.interpreter->output(0);
    if (output->dims->size != 2 || output->dims->data[0] != 1 || output->dims->data[1] != 1 ||
        output->type != kTfLiteUInt8) {
        error_ = "Modell-Ausgang passt nicht (erwartet uint8 [1, 1])";
        return false;
    }
    d.stride = input->dims->data[1];
    return true;
}

void WakeWordDetector::release() {
    if (impl_) {
        if (impl_->interpreter) {
            impl_->interpreter->~MicroInterpreter();
            heap_caps_free(impl_->interpreter);
        }
        if (impl_->frontendReady) FrontendFreeStateContents(&impl_->frontendState);
        if (impl_->arena) heap_caps_free(impl_->arena);
        if (impl_->varArena) heap_caps_free(impl_->varArena);
        impl_->~Impl();
        heap_caps_free(impl_);
        impl_ = nullptr;
    }
    if (model_) {
        heap_caps_free(model_);
        model_ = nullptr;
    }
}

void WakeWordDetector::taskEntry(void *arg) {
    static_cast<WakeWordDetector *>(arg)->run();
}

void WakeWordDetector::run() {
    Impl &d = *impl_;
    const size_t stepSamples = settings_.featureStepMs * SAMPLES_PER_MS;
    size_t filled = 0;
    int8_t features[FEATURE_SIZE];

    while (!stop_) {
        if (recordingReleaseRequested_) {
            recordingReleaseRequested_ = false;
            recordingComplete_ = false;
            d.recordingActive = false;
            d.recordingSamples = 0;
            d.ignoreSlices = -MIN_SLICES_BEFORE_DETECTION;
            mic_->setStreamingEnabled(true);
        }
        if (recordingComplete_) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (d.recordingActive && millis() - d.recordingStartedMs >= MAX_RECORDING_SECONDS * 1000) {
            d.recordingActive = false;
            recordingHasSpeech_ = d.speechSeen;
            recordingComplete_ = true;
            mic_->setStreamingEnabled(false);
            Serial.println("[WAKE] Sprachaufnahme beendet: Maximaldauer erreicht.");
            continue;
        }

        const size_t got = mic_->read(d.samples + filled, stepSamples - filled, pdMS_TO_TICKS(100));
        if (got == 0) {
            if (!mic_->isRunning()) vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        filled += got;
        if (filled < stepSamples) continue;
        filled = 0;

        if (recordingRequested_ && !d.recordingActive) {
            recordingRequested_ = false;
            d.recordingActive = true;
            d.recordingSamples = 0;
            d.recordingStartedMs = millis();
            d.silenceStartedMs = 0;
            d.speechSeen = false;
            Serial.println("[WAKE] Sprachaufnahme gestartet (max. 10 s).");
        }
        if (d.recordingActive) {
            const size_t copyCount = min(stepSamples, MAX_RECORDING_SAMPLES - d.recordingSamples);
            memcpy(d.recording + d.recordingSamples, d.samples, copyCount * sizeof(int16_t));
            d.recordingSamples += copyCount;

            double sumSquares = 0.0;
            for (size_t i = 0; i < stepSamples; ++i) {
                const double sample = d.samples[i] / 32768.0;
                sumSquares += sample * sample;
            }
            const float rms = sqrtf(static_cast<float>(sumSquares / stepSamples));
            const float frameDbfs = rms > 0.0f ? 20.0f * log10f(rms) : -60.0f;
            const uint32_t now = millis();
            if (frameDbfs > SPEECH_GATE_DBFS) {
                d.speechSeen = true;
                d.silenceStartedMs = 0;
            } else if (d.speechSeen && d.silenceStartedMs == 0) {
                d.silenceStartedMs = now;
            }

            const bool silenceEnded = d.speechSeen && d.silenceStartedMs != 0 &&
                now - d.silenceStartedMs >= settings_.endOfSpeechSilenceMs;
            const bool noSpeech = !d.speechSeen &&
                now - d.recordingStartedMs >= NO_SPEECH_GRACE_MS + settings_.endOfSpeechSilenceMs;
            const bool maxDurationReached = d.recordingSamples >= MAX_RECORDING_SAMPLES;
            if (silenceEnded || noSpeech || maxDurationReached) {
                d.recordingActive = false;
                recordingHasSpeech_ = d.speechSeen;
                recordingComplete_ = true;
                mic_->setStreamingEnabled(false);
                Serial.printf("[WAKE] Sprachaufnahme beendet: %u ms, %s\n",
                              (unsigned)((d.recordingSamples * 1000) / MicManager::SAMPLE_RATE),
                              silenceEnded ? "Stille erkannt" : (noSpeech ? "keine Sprache erkannt" : "Maximaldauer erreicht"));
                continue;
            }
        }

        size_t used = 0;
        FrontendOutput out = FrontendProcessSamples(&d.frontendState, d.samples, stepSamples, &used);
        if (out.size != FEATURE_SIZE) continue;

        for (size_t i = 0; i < FEATURE_SIZE; ++i) {
            // Skalierung wie beim Training: (feature * 256) / (25.6 * 26.0) - 128
            int32_t value = ((static_cast<int32_t>(out.values[i]) * 256) + 333) / 666 - 128;
            if (value < -128) value = -128;
            if (value > 127) value = 127;
            features[i] = static_cast<int8_t>(value);
        }
        if (!processFeatures(features)) {
            error_ = "Modell-Ausfuehrung fehlgeschlagen";
            break;
        }
    }
    task_ = nullptr;
    vTaskDelete(nullptr);
}

bool WakeWordDetector::processFeatures(const int8_t *features) {
    Impl &d = *impl_;
    if (d.recordingActive || recordingComplete_) return true;
    if (d.ignoreSlices < 0) ++d.ignoreSlices;

    TfLiteTensor *input = d.interpreter->input(0);
    memcpy(input->data.int8 + FEATURE_SIZE * d.strideStep, features, FEATURE_SIZE);
    if (++d.strideStep < d.stride) return true;
    d.strideStep = 0;

    if (d.interpreter->Invoke() != kTfLiteOk) return false;
    d.probabilityIndex = (d.probabilityIndex + 1) % settings_.slidingWindowSize;
    d.probabilities[d.probabilityIndex] = d.interpreter->output(0)->data.uint8[0];

    uint32_t sum = 0;
    for (size_t i = 0; i < settings_.slidingWindowSize; ++i) sum += d.probabilities[i];
    const float average = static_cast<float>(sum) / (255.0f * settings_.slidingWindowSize);

    portENTER_CRITICAL(&mux_);
    if (average > peak_) peak_ = average;
    portEXIT_CRITICAL(&mux_);

    if (d.ignoreSlices >= 0 && average > settings_.probabilityCutoff) {
        detected_ = true;
        detections_ = detections_ + 1;
        memset(d.probabilities, 0, sizeof(d.probabilities));
        d.ignoreSlices = -MIN_SLICES_BEFORE_DETECTION;
    }
    return true;
}

bool WakeWordDetector::takeDetection() {
    if (!detected_) return false;
    detected_ = false;
    return true;
}

void WakeWordDetector::startRecording() {
    if (!recordingComplete_) recordingRequested_ = true;
}

const int16_t *WakeWordDetector::recordingData() const {
    return impl_ ? impl_->recording : nullptr;
}

size_t WakeWordDetector::recordingSampleCount() const {
    return impl_ ? impl_->recordingSamples : 0;
}

void WakeWordDetector::releaseRecording() {
    recordingReleaseRequested_ = true;
}

float WakeWordDetector::takePeakProbability() {
    portENTER_CRITICAL(&mux_);
    const float value = peak_;
    peak_ = 0.0f;
    portEXIT_CRITICAL(&mux_);
    return value;
}
