#pragma once

enum class WakeWordError {
    None,
    Microphone,
    NoSelection,
    NoSd,
    ModelMissing,
    ModelSize,
    ModelMemory,
    ModelRead,
    InvalidSettings,
    ManifestInvalid,
    SettingsMissing,
    TemporaryCreate,
    JsonWrite,
    JsonBackup,
    JsonActivate,
    SilenceSave,
    TaskStart,
    Memory,
    Frontend,
    Operations,
    Schema,
    ArenaMemory,
    Resources,
    InterpreterMemory,
    Tensors,
    Input,
    Output,
    Execution
};

inline const char *wakeWordErrorEnglish(WakeWordError error) {
    switch (error) {
        case WakeWordError::None: return "";
        case WakeWordError::Microphone: return "Microphone: ";
        case WakeWordError::NoSelection: return "No wake word selected";
        case WakeWordError::NoSd: return "No SD card";
        case WakeWordError::ModelMissing: return "Model file missing";
        case WakeWordError::ModelSize: return "Model file has invalid size";
        case WakeWordError::ModelMemory: return "Out of memory for model";
        case WakeWordError::ModelRead: return "Model file could not be read";
        case WakeWordError::InvalidSettings: return "Invalid settings";
        case WakeWordError::ManifestInvalid: return "Model JSON missing or invalid";
        case WakeWordError::SettingsMissing: return "Settings missing in model JSON";
        case WakeWordError::TemporaryCreate: return "Temporary JSON could not be created";
        case WakeWordError::JsonWrite: return "JSON could not be written completely";
        case WakeWordError::JsonBackup: return "Original JSON could not be backed up";
        case WakeWordError::JsonActivate: return "New JSON could not be activated";
        case WakeWordError::SilenceSave: return "Silence time could not be saved";
        case WakeWordError::TaskStart: return "Task could not be started";
        case WakeWordError::Memory: return "Out of memory";
        case WakeWordError::Frontend: return "Audio frontend could not be initialized";
        case WakeWordError::Operations: return "TFLM operations could not be registered";
        case WakeWordError::Schema: return "Model schema is not supported";
        case WakeWordError::ArenaMemory: return "Out of memory for tensor arena";
        case WakeWordError::Resources: return "Resource variables could not be created";
        case WakeWordError::InterpreterMemory: return "Out of memory for interpreter";
        case WakeWordError::Tensors: return "Tensors could not be allocated (tensor_arena_size too small?)";
        case WakeWordError::Input: return "Model input does not match (expected int8 [1, n, 40])";
        case WakeWordError::Output: return "Model output does not match (expected uint8 [1, 1])";
        case WakeWordError::Execution: return "Model execution failed";
    }
    return "Unknown wake word error";
}
