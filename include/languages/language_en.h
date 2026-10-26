#pragma once

constexpr const char* SD_NOT_AVAILABLE = "SD card not available";
constexpr const char* SD_FILE_NOT_FOUND = "SD file not found";
constexpr const char* SD_PLAYBACK_FAILED = "Playback failed";
constexpr const char* SD_FILE_ALREADY_EXISTS = "File already exists; please use a different name";
constexpr const char* SD_FILE_INVALID = "Invalid filename (MP3, WAV, AAC or FLAC required)";
constexpr const char* SD_FILE_TOO_LARGE = "File too large (max. 20 MB)";
constexpr const char* SD_FILE_WRITE_FAILED = "Write failed (SD full?)";
constexpr const char* SD_FILE_UPLOAD_ABORTED = "Upload aborted";
constexpr const char* SD_FILE_UPLOAD_INCOMPLETE = "Upload incomplete or empty";
constexpr const char* SD_FILE_UPLOAD_FAILED = "Upload failed";
constexpr const char* STATION_LIST_WRITE_FAILED = "Failed to write station list";
constexpr const char* INVALID_STATION_ID = "Invalid station ID";
constexpr const char* INVALID_PARAMETERS = "Invalid parameters";
constexpr const char* NO_M3U_STATIONS_SAVED = "No M3U stations saved";
constexpr const char* NO_LAST_SELECTED_STATION = "No station selected";
constexpr const char* ALARM_SD_FILE_NOT_AVAILABLE = "Selected SD file not available";
constexpr const char* STATION_LIST_CLEAR_FAILED = "Failed to clear station list";
constexpr const char* FAVORITES_LIMIT_PREFIX = "Maximum ";
constexpr const char* FAVORITES_LIMIT_SUFFIX = " favorites";
constexpr const char* INVALID_FAVORITE = "Invalid favorite";
constexpr const char* INVALID_AUDIO_PATH_OR_NO_SD = "Invalid audio path or no SD card";
constexpr const char* FILE_NOT_FOUND = "File not found";
constexpr const char* SD_FILE_IN_USE = "File is playing or selected as alarm sound";
constexpr const char* SD_FILE_DELETE_FAILED = "Delete failed";
constexpr const char* SD_UPLOAD_ONE_FILE_ONLY = "Only one file per upload allowed";
constexpr const char* NO_SD_CARD = "No SD card";
constexpr const char* SD_ALARM_DIR_CREATE_FAILED = "Could not create folder /alarm";
constexpr const char* SD_UPLOAD_TEMP_CREATE_FAILED = "Could not create upload file";
constexpr const char* SD_FILE_SAVE_FAILED = "Could not save file";
constexpr const char* SD_UPLOAD_NO_COMPLETE_FILE = "No complete file received";
constexpr const char* SD_FILE_SAVED_PREFIX = "Saved: ";
constexpr const char* NO_PLAYABLE_SD_FILES = "No playable SD files";
constexpr const char* M3U_BODY_MISSING = "M3U body missing";
constexpr const char* MQTT_HOST_MISSING = "MQTT host missing";
constexpr const char* NO_URL_GIVEN = "No URL given";
constexpr const char* NO_ANNOUNCE_URL_GIVEN = "No announcement URL given";
constexpr const char* INVALID_VALUE = "Invalid value";
constexpr const char* NO_STATION_TO_RESUME = "No station stored to resume";
constexpr const char* MISSING_PARAMETERS = "Missing parameters";
constexpr const char* NOT_FOUND = "Not found";
constexpr const char* STATUS_READY_NO_STATION = "Ready (no station selected)";
constexpr const char* STATUS_NO_TITLE = "No title";
constexpr const char* STATUS_LOCAL_FILE = "Local file";
constexpr const char* STATUS_CONNECTING = "Connecting...";
constexpr const char* STATUS_ANNOUNCEMENT = "📢 ANNOUNCEMENT";
constexpr const char* STATUS_SPEAKING = "Speaking...";
constexpr const char* STATUS_ALARM_TRIGGERED = "⏰ ALARM TRIGGERED";
constexpr const char* STATUS_CUSTOM_STREAM = "Custom stream";
constexpr const char* STATUS_RADIO = "Radio";
constexpr const char* STATUS_SPEECH_CAPTURE = "Speech recording";
constexpr const char* STATUS_RECORDING_SPEECH = "Recording speech...";
constexpr const char* STATUS_SPEECH_COMPLETE = "Speech recording completed";
constexpr const char* STATUS_NO_SPEECH = "No speech detected";
constexpr const char* WAKE_ERROR_MICROPHONE = "Microphone: ";
constexpr const char* WAKE_ERROR_NO_SELECTION = "No wake word selected";
constexpr const char* WAKE_ERROR_NO_SD = "No SD card";
constexpr const char* WAKE_ERROR_MODEL_MISSING = "Model file missing";
constexpr const char* WAKE_ERROR_MODEL_SIZE = "Model file has invalid size";
constexpr const char* WAKE_ERROR_MODEL_MEMORY = "Out of memory for model";
constexpr const char* WAKE_ERROR_MODEL_READ = "Model file could not be read";
constexpr const char* WAKE_ERROR_INVALID_SETTINGS = "Invalid settings";
constexpr const char* WAKE_ERROR_MANIFEST_INVALID = "Model JSON missing or invalid";
constexpr const char* WAKE_ERROR_SETTINGS_MISSING = "Settings missing in model JSON";
constexpr const char* WAKE_ERROR_TEMPORARY_CREATE = "Temporary JSON could not be created";
constexpr const char* WAKE_ERROR_JSON_WRITE = "JSON could not be written completely";
constexpr const char* WAKE_ERROR_JSON_BACKUP = "Original JSON could not be backed up";
constexpr const char* WAKE_ERROR_JSON_ACTIVATE = "New JSON could not be activated";
constexpr const char* WAKE_ERROR_SILENCE_SAVE = "Silence time could not be saved";
constexpr const char* WAKE_ERROR_TASK_START = "Task could not be started";
constexpr const char* WAKE_ERROR_MEMORY = "Out of memory";
constexpr const char* WAKE_ERROR_FRONTEND = "Audio frontend could not be initialized";
constexpr const char* WAKE_ERROR_OPERATIONS = "TFLM operations could not be registered";
constexpr const char* WAKE_ERROR_SCHEMA = "Model schema is not supported";
constexpr const char* WAKE_ERROR_ARENA_MEMORY = "Out of memory for tensor arena";
constexpr const char* WAKE_ERROR_RESOURCES = "Resource variables could not be created";
constexpr const char* WAKE_ERROR_INTERPRETER_MEMORY = "Out of memory for interpreter";
constexpr const char* WAKE_ERROR_TENSORS = "Tensors could not be allocated (tensor_arena_size too small?)";
constexpr const char* WAKE_ERROR_INPUT = "Model input does not match (expected int8 [1, n, 40])";
constexpr const char* WAKE_ERROR_OUTPUT = "Model output does not match (expected uint8 [1, 1])";
constexpr const char* WAKE_ERROR_EXECUTION = "Model execution failed";
constexpr const char* WAKE_ERROR_UNKNOWN = "Unknown wake word error";
constexpr const char* HA_RADIO_POWER = "Radio Power";
constexpr const char* HA_RADIO_VOLUME = "Radio Volume";
constexpr const char* HA_RADIO_STATION = "Radio Station";
constexpr const char* HA_RADIO_TITLE = "Radio Title";
constexpr const char* HA_STATION_SELECT = "Radio Station Select";
constexpr const char* HA_SD_SELECT = "SD File Select";
constexpr const char* HA_ALARM_ENABLED = "Alarm Enabled";
constexpr const char* HA_ALARM_TIME = "Alarm Time";
constexpr const char* HA_ALARM_VOLUME = "Alarm Volume";
constexpr const char* HA_ALARM_SOURCE = "Alarm Source";
constexpr const char* HA_ALARM_SD_FILE = "Alarm SD File";
constexpr const char* HA_ANNOUNCE_VOLUME = "Announcement Volume";
constexpr const char* HA_ANNOUNCE_STATUS = "Announcement Status";
#define T_ALARM_LABEL_SD_FILE_PREFIX "SD file: "
constexpr const char* ALARM_LABEL_SD_FILE_PREFIX = T_ALARM_LABEL_SD_FILE_PREFIX;
constexpr const char* ALARM_LABEL_RADIO_PREFIX = "Radio: ";
constexpr const char* ALARM_LABEL_RADIO_NOT_SET = "Radio: not set";

// Web interface texts (string macros, concatenated at compile time in web_pages.h).
// Must not contain quotes, backslashes or line breaks (some are used inside JavaScript strings).
#define T_HTML_LANG "en"
#define T_LOADING "Loading..."
#define T_ALARM_PAGE_TITLE "Alarm - ESP32 Radio"
#define T_ALARM_HEADING "⏰ Radio Alarm"
#define T_ALARM_SUBTITLE "Real-time clock & alarm settings"
#define T_ALARM_CLOCK_LABEL "ESP32 time (NTP)"
#define T_ALARM_CLOCK_SUFFIX ""
#define T_ALARM_CURRENT_SOUND "Current alarm sound"
#define T_ALARM_TIME_LABEL "Alarm time"
#define T_ALARM_ENABLE "Enable alarm"
#define T_ALARM_VOLUME_LABEL "Alarm volume"
#define T_ALARM_SAVE_BUTTON "💾 Save alarm"
#define T_ALARM_NEED_STATIONS_MAIN "Please upload an M3U station list on the main page first."
#define T_ALARM_TEST_HEADING "Test"
#define T_ALARM_TEST_BUTTON "🔔 Test alarm now"
#define T_ALARM_SAVED_MSG "✅ Alarm settings saved!"
#define T_ALARM_TRIGGERED_MSG "🔔 Alarm signal triggered!"
#define T_ALARM_STOP_TEST_BUTTON "⏹ Stop alarm test"
#define T_ALARM_STOPPED_MSG "⏹ Alarm test stopped."
#define T_ALARM_NEED_STATIONS "Please upload an M3U station list first."

#define T_SD_PAGE_TITLE "MicroSD - ESP32 Radio"
#define T_SD_HEADING "MicroSD files"
#define T_SD_INTRO "Play local audio files or save them as the alarm source."
#define T_SD_PLAY_ALL "▶ Play all"
#define T_SD_STOP_ALL "■ Stop all"
#define T_SD_REFRESH "↻ Refresh"
#define T_SD_UPLOAD_HEADING "Upload alarm sound"
#define T_SD_FILE_ARIA "Select alarm sound"
#define T_SD_UPLOAD_BUTTON "Upload alarm sound"
#define T_SD_FILES_HEADING "Audio files"
#define T_SD_LOADING_FILES "Loading files ..."
#define T_BACK_TO_RADIO "Back to radio"
#define T_SD_ACTION_FAILED "Action failed"
#define T_SD_PLAY_TITLE "Play on the device"
#define T_SD_PLAY_STARTED "Playback started."
#define T_SD_SET_ALARM_TITLE "Save as alarm sound"
#define T_SD_ALARM_SAVED "File saved as alarm source."
#define T_SD_DOWNLOAD_TITLE "Download"
#define T_SD_DELETE_TITLE "Delete"
#define T_SD_DELETE_CONFIRM_PREFIX "Delete file "
#define T_SD_DELETE_CONFIRM_SUFFIX "?"
#define T_SD_DELETED "File deleted."
#define T_SD_LIST_UNREACHABLE "SD list not reachable"
#define T_SD_NO_FILES "No playable files found or no SD card inserted."
#define T_SD_FILES_FOUND_PREFIX ""
#define T_SD_FILES_FOUND_SUFFIX " file(s) found."
#define T_SD_LOAD_FAILED "Could not load SD files."
#define T_SD_UPLOAD_SIZE_HINT "Please select a file between 1 byte and 20 MB."
#define T_SD_UPLOAD_RUNNING "Upload running..."
#define T_SD_PLAY_ALL_MSG "All files will be played in sequence."
#define T_SD_STOPPED "Playback stopped."

#define T_SETUP_PAGE_TITLE "Wi-Fi configuration - ESP32 Radio"
#define T_SETUP_HEADING "⚙️ Wi-Fi settings"
#define T_SETUP_SUBTITLE "Connect your ESP32 Radio to your home network"
#define T_SETUP_CONNECTED_PREFIX "Connected to "
#define T_SETUP_CONNECTED_SUFFIX ""
#define T_SETUP_NETWORKS_LABEL "Available networks (2.4 GHz)"
#define T_SETUP_SCAN_BUTTON "🔄 Scan"
#define T_SETUP_SCAN_HINT "Click Scan to search for networks"
#define T_SETUP_MANUAL_SSID_LABEL "Or enter SSID manually"
#define T_SETUP_SSID_PLACEHOLDER "Wi-Fi name (SSID)"
#define T_SETUP_WIFI_PASS_LABEL "Wi-Fi password"
#define T_SETUP_WIFI_PASS_PLACEHOLDER "Enter password"
#define T_SETUP_SAVE_CONNECT "💾 Save & connect"
#define T_SETUP_OPTIONS_HEADING "Options"
#define T_SETUP_RESET_BUTTON "🗑️ Delete saved Wi-Fi credentials"
#define T_SETUP_MQTT_HOST_LABEL "Broker address"
#define T_SETUP_MQTT_USER_LABEL "Username (optional)"
#define T_SETUP_MQTT_LEAVE_EMPTY "can be left empty"
#define T_SETUP_MQTT_PASS_LABEL "Password (optional)"
#define T_SETUP_MQTT_BASE_LABEL "MQTT topic base"
#define T_SETUP_MQTT_SAVE "💾 Save settings"
#define T_SETUP_MQTT_STATUS_LOADING "Loading MQTT status..."
#define T_SETUP_SEARCHING_NETWORKS "Searching networks..."
#define T_SETUP_SCANNING "Scanning Wi-Fi networks..."
#define T_SETUP_SELECT_NETWORK "-- Select network --"
#define T_SETUP_NO_NETWORKS "No networks found"
#define T_SETUP_NETWORKS_FOUND_PREFIX ""
#define T_SETUP_NETWORKS_FOUND_SUFFIX " networks found."
#define T_SETUP_SCAN_ERROR "Error while scanning networks."
#define T_SETUP_SSID_REQUIRED "Please enter a Wi-Fi name (SSID)."
#define T_SETUP_SAVING_PREFIX "Saving and connecting to "
#define T_SETUP_SAVING_SUFFIX "..."
#define T_SETUP_SAVED "✅ Saved! The ESP32 is reconnecting now..."
#define T_SETUP_SAVED_HOTSPOT_HINT "If you were in hotspot mode, reconnect your phone to your normal Wi-Fi now."
#define T_SETUP_SAVE_ERROR "Error while saving the data."
#define T_SETUP_RESET_CONFIRM "Do you really want to delete the saved Wi-Fi credentials?"
#define T_SETUP_RESET_DONE "Wi-Fi credentials deleted. Restarting..."
#define T_SETUP_MQTT_DISABLED "MQTT disabled"
#define T_SETUP_MQTT_CONNECTED_PREFIX "MQTT connected to "
#define T_SETUP_MQTT_CONNECTED_SUFFIX ""
#define T_SETUP_MQTT_DISCONNECTED_PREFIX "MQTT disconnected from "
#define T_SETUP_MQTT_DISCONNECTED_SUFFIX ""
#define T_SETUP_MQTT_UNKNOWN "MQTT status unknown"
#define T_SETUP_MQTT_UNREACHABLE "MQTT status not reachable"
#define T_SETUP_MQTT_SAVED "MQTT saved. Connecting."
#define T_SETUP_WAV_TRANSPORT_LABEL "Transfer method for WAV"
#define T_SETUP_WAV_MQTT_TOPIC_LABEL "MQTT topic for WAV messages"
#define T_SETUP_WAV_HTTP_URL_LABEL "HTTP URL for WAV messages"
#define T_SETUP_WAV_HTTP_HINT "Only http:// is supported. Use the IP address of the Whisper server, not localhost."
#define T_SETUP_HTTP_URL_REQUIRED "Please enter an HTTP URL starting with http://."
#define T_SETUP_SAVE_FAILED_PREFIX "Saving failed: "

#define T_IDX_OPTIONS "Options"
#define T_IDX_NAV_ALARM "⏰ Alarm"
#define T_IDX_NAV_WIFI "⚙️ Wi-Fi"
#define T_IDX_STOPPED "Stopped"
#define T_IDX_BITRATE_LABEL "Bitrate: "
#define T_IDX_VOLUME "Volume"
#define T_IDX_STOP_BUTTON "⏹ Stop"
#define T_IDX_PLAY_BUTTON "▶ Play"
#define T_IDX_FAVORITES_HEADING "Favorites"
#define T_IDX_SHOW_ALL "Show all stations"
#define T_IDX_SAVE_AS_ALARM "⏰ Save as alarm sound"
#define T_IDX_STATION_LIST_HEADING "Station list"
#define T_IDX_SEARCH_PLACEHOLDER "Search stations..."
#define T_IDX_CUSTOM_URL_HEADING "Custom stream URL"
#define T_IDX_CUSTOM_PLAY "Play"
#define T_IDX_ANNOUNCE_VOLUME "📢 Announcement volume (TTS)"
#define T_IDX_M3U_HEADING "M3U station list"
#define T_IDX_M3U_APPEND "Append to existing list"
#define T_IDX_M3U_REPLACE "Replace existing list"
#define T_IDX_M3U_UPLOAD "📂 Upload M3U"
#define T_IDX_CLEAR_ALL "🗑️ Delete all stations"
#define T_IDX_NO_FAVORITES "No favorites yet – mark stations with ☆ in the station list."
#define T_IDX_LOAD_ERROR "Loading error"
#define T_IDX_NO_MATCHES "No matches."
#define T_IDX_NO_STATIONS_PREFIX "No stations stored – upload an M3U list under "
#define T_IDX_NO_STATIONS_SUFFIX "."
#define T_IDX_PLAY_TITLE "Play"
#define T_IDX_FAV_REMOVE "Remove from favorites"
#define T_IDX_FAV_ADD "Add to favorites"
#define T_IDX_DELETE_TITLE "Delete"
#define T_IDX_PAGE_OF " of "
#define T_IDX_NO_SD_LIMIT_SHORT " (max. 24 without SD card)"
#define T_IDX_FAV_CHANGE_FAILED "Could not change favorite: "
#define T_IDX_DELETE_CONFIRM_PREFIX "Delete station "
#define T_IDX_DELETE_CONFIRM_SUFFIX "?"
#define T_IDX_STATION_DELETED "Station deleted."
#define T_IDX_DELETE_FAILED "Delete failed: "
#define T_IDX_CLEAR_CONFIRM "Really delete ALL stations? Favorites are kept."
#define T_IDX_CLEARED "All stations deleted."
#define T_IDX_M3U_SELECT_FILE "Please select an M3U file first."
#define T_IDX_M3U_UPLOADING "Uploading M3U..."
#define T_IDX_IMPORTED_PREFIX ""
#define T_IDX_IMPORTED_MID " stations imported, "
#define T_IDX_IMPORTED_SUFFIX " in total."
#define T_IDX_NVS_LIMIT_NOTE " Without an SD card, at most 24 stations are stored."
#define T_IDX_UPLOAD_FAILED "Upload failed: "
#define T_IDX_ALARM_STATION_SAVED "The current station is now the alarm sound."
#define T_IDX_ALARM_STATION_FAILED "Could not save alarm sound: "
#define T_IDX_UNKNOWN "Unknown"
#define T_IDX_BADGE_ANNOUNCING "📢 Announcement"
#define T_IDX_BADGE_PLAYING "Playing"

#define T_IDX_SPEECH_RECORDING "Speech recording in progress"
#define T_IDX_WAKE_WORD_SETTINGS_MISSING "Wake-Word settings missing"
#define T_IDX_WAKE_WORD_SETTINGS_INVALID "Invalid Wake-Word settings"
#define T_IDX_WAKE_WORD_MODEL_NOT_FOUND "Wake-Word model not found"
#define T_IDX_I2S_MIC_START_FAILED "I2S microphone could not be started: "

#define T_IDX_SUBTITLE "Audio Streamer"
#define T_SETUP_PORT_LABEL "Port"
#define T_ALARM_STATION_LIST_UNAVAILABLE "Station list unavailable"

#define T_WW_NAME "Wake Word"
#define T_WW_MODELS_INTRO "microWakeWord models"
#define T_WW_FOLDER_INTRO "in the folder"
#define T_WW_SD_INTRO "on the MicroSD card."
#define T_WW_ACTIVE "Active"
#define T_WW_DETECTION_LABEL "Detection: "
#define T_WW_PROBABILITY_LABEL "Probability: "
#define T_WW_DISABLE "Disable wake word"
#define T_WW_LOADING_MODELS "Loading models ..."
#define T_WW_DETECTION_HEADING "Wake word detection"
#define T_WW_THRESHOLD "Threshold"
#define T_WW_THRESHOLD_HINT "Higher values require greater confidence in wake word detection."
#define T_WW_WINDOW_SIZE "Window size"
#define T_WW_WINDOW_HINT "Number of evaluations used for averaging."
#define T_WW_SILENCE_LABEL "Silence before end of speech"
#define T_WW_SILENCE_HINT "Silence after speaking before the recording ends."
#define T_WW_SAVE_SETTINGS "Save wake word settings"
#define T_WW_MISSING_FILE_PREFIX "Missing "
#define T_WW_MISSING_FILE_SUFFIX ""
#define T_WW_USE "Use"
#define T_WW_NONE "none"
#define T_WW_NO_SD "No MicroSD card inserted."
#define T_WW_SD_UNAVAILABLE "MicroSD card unavailable."
#define T_WW_FOLDER_FAILED_PREFIX "Could not create folder "
#define T_WW_FOLDER_FAILED_SUFFIX "."
#define T_WW_FOLDER_ERROR "Folder error on the MicroSD card."
#define T_WW_NO_MODELS_PREFIX "No models found. Copy name.tflite and name.json to "
#define T_WW_LOAD_FAILED "Could not load models."
#define T_WW_SELECT_FAILED "Selection failed"
#define T_WW_SELECTED_PREFIX "Wake word "
#define T_WW_SELECTED_SUFFIX " saved."
#define T_WW_DISABLED "Wake word disabled."
#define T_WW_RUNNING "running"
#define T_WW_OFF "off"
#define T_WW_DETECTED_LABEL "detections: "
#define T_WW_DETECTED "Wake word detected!"
#define T_WW_STATUS_UNREACHABLE "Detection: status unreachable"
#define T_WW_SETTINGS_SAVED "Settings saved and detection restarted."
#define T_WW_SERVER_UNREACHABLE "Server unreachable"

#define T_SETUP_WAV_WYOMING_LABEL "Wyoming settings"