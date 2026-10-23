#pragma once
#include "wake_word_error.h"

#define LANG_DE 1
#define LANG_EN 2
#define LANG_PL 3

#ifndef UI_LANG
#define UI_LANG LANG_DE
#endif

#if UI_LANG == LANG_DE
    #include "languages/language_de.h"
#elif UI_LANG == LANG_EN
    #include "languages/language_en.h"
#elif UI_LANG == LANG_PL
    #include "languages/language_pl.h"
#else
    #error "Unknown language"
#endif

inline const char *localizedWakeWordError(WakeWordError error) {
    switch (error) {
        case WakeWordError::None: return "";
        case WakeWordError::Microphone: return WAKE_ERROR_MICROPHONE;
        case WakeWordError::NoSelection: return WAKE_ERROR_NO_SELECTION;
        case WakeWordError::NoSd: return WAKE_ERROR_NO_SD;
        case WakeWordError::ModelMissing: return WAKE_ERROR_MODEL_MISSING;
        case WakeWordError::ModelSize: return WAKE_ERROR_MODEL_SIZE;
        case WakeWordError::ModelMemory: return WAKE_ERROR_MODEL_MEMORY;
        case WakeWordError::ModelRead: return WAKE_ERROR_MODEL_READ;
        case WakeWordError::InvalidSettings: return WAKE_ERROR_INVALID_SETTINGS;
        case WakeWordError::ManifestInvalid: return WAKE_ERROR_MANIFEST_INVALID;
        case WakeWordError::SettingsMissing: return WAKE_ERROR_SETTINGS_MISSING;
        case WakeWordError::TemporaryCreate: return WAKE_ERROR_TEMPORARY_CREATE;
        case WakeWordError::JsonWrite: return WAKE_ERROR_JSON_WRITE;
        case WakeWordError::JsonBackup: return WAKE_ERROR_JSON_BACKUP;
        case WakeWordError::JsonActivate: return WAKE_ERROR_JSON_ACTIVATE;
        case WakeWordError::SilenceSave: return WAKE_ERROR_SILENCE_SAVE;
        case WakeWordError::TaskStart: return WAKE_ERROR_TASK_START;
        case WakeWordError::Memory: return WAKE_ERROR_MEMORY;
        case WakeWordError::Frontend: return WAKE_ERROR_FRONTEND;
        case WakeWordError::Operations: return WAKE_ERROR_OPERATIONS;
        case WakeWordError::Schema: return WAKE_ERROR_SCHEMA;
        case WakeWordError::ArenaMemory: return WAKE_ERROR_ARENA_MEMORY;
        case WakeWordError::Resources: return WAKE_ERROR_RESOURCES;
        case WakeWordError::InterpreterMemory: return WAKE_ERROR_INTERPRETER_MEMORY;
        case WakeWordError::Tensors: return WAKE_ERROR_TENSORS;
        case WakeWordError::Input: return WAKE_ERROR_INPUT;
        case WakeWordError::Output: return WAKE_ERROR_OUTPUT;
        case WakeWordError::Execution: return WAKE_ERROR_EXECUTION;
    }
    return WAKE_ERROR_UNKNOWN;
}
