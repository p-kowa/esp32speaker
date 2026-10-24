#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Audio.h"
#include "config.h"
#include "web_pages.h"
#include "wifi_manager.h"
#include "alarm_manager.h"
#include "station_manager.h"
#include "mqtt_manager.h"
#include "sd_manager.h"
#include "language.h"
#include <vector>

// Stack reserve for web server uploads (M3U import) and station file access in the loop task
SET_LOOP_TASK_STACK_SIZE(16 * 1024);

// Instances
Audio audio;
WebServer server(80);
WiFiConfigManager wifiMgr;
AlarmManager alarmMgr;
StationManager stationMgr;
MqttManager mqttMgr;
SdManager sdMgr;

// State variables
// Language-independent marker: the code detects SD playback via this station name prefix
constexpr const char *SD_STATION_PREFIX = "SD: ";
String currentStation = STATUS_READY_NO_STATION;
String currentTitle = STATUS_NO_TITLE;
String currentBitrate = "--";
String currentStreamUrl = "";
int currentVolume = DEFAULT_VOLUME;
int currentAnnounceVolume = DEFAULT_ANNOUNCE_VOLUME;
bool isPlaying = false;
bool radioPowerOn = true;
bool shouldReboot = false;
unsigned long rebootTimer = 0;
Preferences audioPrefs;
std::vector<String> sdPlayAllPaths;
size_t sdPlayAllIndex = 0;
bool sdPlayAllActive = false;
// True while an alarm (real or test) is the current playback; cleared whenever other playback starts
bool alarmPlayback = false;
// Set by audio callbacks, handled in loop() (no restart from within audio.loop())
volatile bool pendingResume = false;
volatile bool pendingSdNext = false;

// Snapshot and announcement (announcement / TTS) state
struct PlaybackSnapshot {
    bool wasPlaying = false;
    String station = "";
    String url = "";
    String title = "";
    int volume = DEFAULT_VOLUME;
    bool isSd = false;
    String sdPath = "";
    bool sdPlayAll = false;
    size_t sdPlayAllIdx = 0;
    std::vector<String> sdPlayAllList;
};

PlaybackSnapshot previousState;
bool isAnnouncing = false;
unsigned long announceStartMs = 0;
unsigned long announceLastActiveMs = 0;
const unsigned long ANNOUNCE_IDLE_TIMEOUT_MS = 10000;
const unsigned long ANNOUNCE_MAX_MS = 300000;

void stopPlayback();
bool alarmPlaybackActive() { return alarmPlayback && audio.isRunning(); }void triggerAlarmPlayback();
void clearSdPlayAll();
void playAnnouncement(const String &url, int volume = -1);
void resumeAfterAnnouncement();

String buildStationOptionsJson() {
    return stationMgr.favNamesJson();
}

String buildSdOptionsJson() {
    String json = "[";
    for (const String &path : sdMgr.paths()) {
        String escaped = path;
        escaped.replace("\\", "\\\\");
        escaped.replace("\"", "\\\"");
        if (json.length() > 1) json += ',';
        json += "\"" + escaped + "\"";
    }
    json += ']';
    return json;
}

void savePersistedRadioPower(bool powerOn) {
    if (radioPowerOn == powerOn) return;
    radioPowerOn = powerOn;
    audioPrefs.begin("audio_config", false);
    audioPrefs.putBool("radio_on", powerOn);
    audioPrefs.end();
}

bool startSdPath(const String &path, bool persistPower = true) {
    if (!sdMgr.isMounted() || !sdMgr.exists(path)) return false;
    alarmPlayback = false;
    audio.stopSong();
    currentStation = String(SD_STATION_PREFIX) + path;
    currentTitle = STATUS_LOCAL_FILE;
    currentBitrate = "--";
    currentStreamUrl = path;
    audio.setVolume(currentVolume);
    isPlaying = audio.connecttoFS(sdMgr.filesystem(), path.c_str());
    if (persistPower) savePersistedRadioPower(true);
    return isPlaying;
}

void clearSdPlayAll() {
    sdPlayAllPaths.clear();
    sdPlayAllIndex = 0;
    sdPlayAllActive = false;
}

void startStream(const String &name, const String &url, bool persistPower = true) {
    if (url.isEmpty()) return;
    clearSdPlayAll();
    alarmPlayback = false;
    currentStation = name;
    currentTitle = STATUS_CONNECTING;
    currentBitrate = "--";
    currentStreamUrl = url;
    isPlaying = true;
    audio.connecttohost(currentStreamUrl.c_str());
    audio.setVolume(currentVolume);
    if (persistPower) savePersistedRadioPower(true);
}

void playStoredStation(const StoredStation &station) {
    stationMgr.setLastSelected(station);
    startStream(station.name, station.url);
}

bool startLastOrFirst() {
    StoredStation station;
    if (!stationMgr.getLastSelected(station) && !stationMgr.favGet(0, station) && !stationMgr.get(0, station)) {
        return false;
    }
    startStream(station.name, station.url);
    return true;
}

void playNextSdFile() {
    if (!sdPlayAllActive) return;
    ++sdPlayAllIndex;
    if (sdPlayAllIndex < sdPlayAllPaths.size()) {
        isPlaying = startSdPath(sdPlayAllPaths[sdPlayAllIndex], false);
    } else {
        clearSdPlayAll();
        isPlaying = false;
    }
}

void loadPersistedVolume() {
    audioPrefs.begin("audio_config", true);
    currentVolume = constrain(audioPrefs.getInt("volume", DEFAULT_VOLUME), 0, 21);
    currentAnnounceVolume = constrain(audioPrefs.getInt("ann_vol", DEFAULT_ANNOUNCE_VOLUME), 0, 21);
    radioPowerOn = audioPrefs.getBool("radio_on", true);
    audioPrefs.end();
}

void savePersistedVolume() {
    audioPrefs.begin("audio_config", false);
    audioPrefs.putInt("volume", currentVolume);
    audioPrefs.end();
}

void savePersistedAnnounceVolume() {
    audioPrefs.begin("audio_config", false);
    audioPrefs.putInt("ann_vol", currentAnnounceVolume);
    audioPrefs.end();
}

void setRadioVolume(int value, bool persist = true) {
    currentVolume = constrain(value, 0, 21);
    audio.setVolume(currentVolume);
    if (persist) savePersistedVolume();
}

void setAnnounceVolume(int value) {
    currentAnnounceVolume = constrain(value, 0, 21);
    savePersistedAnnounceVolume();
    mqttMgr.publishState(audio.isRunning(), currentStation, currentTitle, currentVolume, currentAnnounceVolume, isAnnouncing);
}

void playAnnouncement(const String &url, int volume) {
    if (url.isEmpty()) return;

    // If an announcement is already running, do not take a new snapshot
    if (!isAnnouncing) {
        previousState.wasPlaying = audio.isRunning() || isPlaying;
        previousState.station = currentStation;
        previousState.title = currentTitle;
        previousState.url = currentStreamUrl;
        previousState.volume = currentVolume;
        previousState.isSd = currentStation.startsWith(SD_STATION_PREFIX);
        previousState.sdPath = previousState.isSd ? currentStreamUrl : "";
        previousState.sdPlayAll = sdPlayAllActive;
        previousState.sdPlayAllIdx = sdPlayAllIndex;
        previousState.sdPlayAllList = sdPlayAllPaths;
    }

    isAnnouncing = true;
    alarmPlayback = false;
    announceStartMs = millis();
    announceLastActiveMs = announceStartMs;

    int targetVol = (volume >= 0) ? constrain(volume, 0, 21) : currentAnnounceVolume;
    audio.stopSong();
    clearSdPlayAll();

    audio.setVolume(targetVol);
    currentStation = STATUS_ANNOUNCEMENT;
    currentTitle = STATUS_SPEAKING;
    currentBitrate = "--";
    currentStreamUrl = url;
    isPlaying = true;

    Serial.printf("[ANNOUNCE] Starting announcement (%s) at volume %d (before: wasPlaying=%d, vol=%d)\n",
                  url.c_str(), targetVol, previousState.wasPlaying, previousState.volume);
    
    mqttMgr.publishState(true, currentStation, currentTitle, targetVol, currentAnnounceVolume, true);
    audio.connecttohost(url.c_str());
}

void resumeAfterAnnouncement() {
    if (!isAnnouncing) return;
    isAnnouncing = false;

    Serial.printf("[ANNOUNCE] Announcement finished. Restoring: wasPlaying=%d, isSd=%d, vol=%d\n",
                  previousState.wasPlaying, previousState.isSd, previousState.volume);

    audio.stopSong();
    audio.setVolume(previousState.volume);
    currentVolume = previousState.volume;

    if (previousState.wasPlaying) {
        if (previousState.isSd) {
            if (previousState.sdPlayAll && !previousState.sdPlayAllList.empty()) {
                sdPlayAllPaths = previousState.sdPlayAllList;
                sdPlayAllIndex = previousState.sdPlayAllIdx;
                sdPlayAllActive = true;
                if (sdPlayAllIndex < sdPlayAllPaths.size()) {
                    startSdPath(sdPlayAllPaths[sdPlayAllIndex], false);
                }
            } else if (!previousState.sdPath.isEmpty()) {
                startSdPath(previousState.sdPath, false);
            }
        } else if (!previousState.url.isEmpty()) {
            startStream(previousState.station.isEmpty() ? String(STATUS_RADIO) : previousState.station, previousState.url, false);
        }
    } else {
        stopPlayback();
    }

    mqttMgr.publishState(audio.isRunning(), currentStation, currentTitle, currentVolume, currentAnnounceVolume, false);
}

void handleMqttCommand(const String &topic, const String &payload) {
    if (topic.endsWith("/set/power")) {
        if (payload == "OFF") stopPlayback();
        else if (payload == "ON") startLastOrFirst();
    } else if (topic.endsWith("/set/volume")) {
        setRadioVolume(payload.toInt());
    } else if (topic.endsWith("/set/station")) {
        StoredStation station;
        if (payload.indexOf("://") >= 0) startStream(STATUS_CUSTOM_STREAM, payload);
        else if (stationMgr.get(payload.toInt(), station)) playStoredStation(station);
    } else if (topic.endsWith("/set/station_url")) {
        startStream(STATUS_CUSTOM_STREAM, payload);
    } else if (topic.endsWith("/set/station_name")) {
        StoredStation station;
        for (size_t i = 0; stationMgr.favGet(i, station); ++i) {
            if (station.name == payload) {
                playStoredStation(station);
                break;
            }
        }
    } else if (topic.endsWith("/set/m3u")) {
        stationMgr.importM3u(payload);
    } else if (topic.endsWith("/set/alarm")) {
        int first = payload.indexOf(':');
        int second = payload.indexOf(':', first + 1);
        int third = payload.indexOf(':', second + 1);
        if (first > 0 && second > first && third > second) {
            alarmMgr.saveSettings(payload.substring(0, first).toInt(),
                                  payload.substring(first + 1, second).toInt(),
                                  payload.substring(second + 1, third).toInt() != 0,
                                  payload.substring(third + 1).toInt());
        }
    } else if (topic.endsWith("/set/alarm_test")) {
        triggerAlarmPlayback();
    } else if (topic.endsWith("/set/alarm_enabled")) {
        alarmMgr.setEnabled(payload == "ON");
    } else if (topic.endsWith("/set/alarm_time")) {
        int colon = payload.indexOf(':');
        if (colon > 0) alarmMgr.setTime(payload.substring(0, colon).toInt(), payload.substring(colon + 1).toInt());
    } else if (topic.endsWith("/set/alarm_volume")) {
        alarmMgr.setVolume(payload.toInt());
        if (alarmPlaybackActive()) setRadioVolume(alarmMgr.getVolume(), false);
    } else if (topic.endsWith("/set/alarm_source")) {
        alarmMgr.setSourceType(payload);
    } else if (topic.endsWith("/set/sd_play")) {
        clearSdPlayAll();
        startSdPath(payload);
    } else if (topic.endsWith("/set/alarm_sd_path")) {
        alarmMgr.saveSource("sd", payload);
    } else if (topic.endsWith("/set/announce_volume")) {
        setAnnounceVolume(payload.toInt());
    } else if (topic.endsWith("/set/announce")) {
        String url = payload;
        int vol = -1;
        // Optional JSON support: {"url":"http://...","volume":15}
        if (payload.startsWith("{") && payload.endsWith("}")) {
            int urlKey = payload.indexOf("\"url\"");
            if (urlKey >= 0) {
                int startQuote = payload.indexOf('"', urlKey + 5);
                int endQuote = payload.indexOf('"', startQuote + 1);
                if (startQuote >= 0 && endQuote > startQuote) {
                    url = payload.substring(startQuote + 1, endQuote);
                }
            }
            int volKey = payload.indexOf("\"volume\"");
            if (volKey >= 0) {
                int colon = payload.indexOf(':', volKey);
                if (colon >= 0) {
                    vol = payload.substring(colon + 1).toInt();
                }
            }
        }
        playAnnouncement(url, vol);
    }
}

void handleDeviceIdGet() {
    char buf[13];
    snprintf(buf, sizeof(buf), "%012llX", static_cast<unsigned long long>(ESP.getEfuseMac()));
    server.send(200, "application/json", "{\"device_id\":\"" + String(buf) + "\"}");
}

// Web server route: main page
void handleRoot() {
    if (wifiMgr.isApMode()) {
        server.send(200, "text/html", SETUP_HTML);
    } else {
        server.send(200, "text/html", INDEX_HTML);
    }
}

// Web server route: setup page
void handleSetup() {
    server.send(200, "text/html", SETUP_HTML);
}

// Web server route: microSD page
void handleSdCard() {
    server.send(200, "text/html", SDCARD_HTML);
}


// Web server route: alarm page
void handleAlarmPage() {
    server.send(200, "text/html", ALARM_HTML);
}

// Web server route: alarm status API
void handleAlarmStatus() {
    String json = alarmMgr.getStatusJson();
    json.remove(json.length() - 1);
    json += ",\"station_count\":" + String(stationMgr.size()) + ",\"alarm_playing\":" + (alarmPlaybackActive() ? "true" : "false") + "}";
    String alarmLabel = STATUS_RADIO;
    if (alarmMgr.getSource() == "sd") {
        alarmLabel = String(ALARM_LABEL_SD_FILE_PREFIX) + alarmMgr.getSdPath();
    } else {
        alarmLabel = alarmMgr.getRadioName().isEmpty() ? String(ALARM_LABEL_RADIO_NOT_SET) : String(ALARM_LABEL_RADIO_PREFIX) + alarmMgr.getRadioName();
    }
    json.remove(json.length() - 1);
    json += ",\"alarm_label\":\"" + StationManager::jsonEscape(alarmLabel) + "\"}";
    server.send(200, "application/json", json);
}

// Helper: trigger alarm playback
void triggerAlarmPlayback() {
    if (alarmMgr.getSource() == "sd") {
        if (!sdMgr.isMounted() || !sdMgr.exists(alarmMgr.getSdPath())) {
            Serial.println("[ALARM] Selected SD file is not available.");
            return;
        }
        clearSdPlayAll();
        setRadioVolume(alarmMgr.getVolume(), false);
        currentTitle = STATUS_ALARM_TRIGGERED;
        isPlaying = startSdPath(alarmMgr.getSdPath(), false);
        alarmPlayback = isPlaying;
        return;
    }
    if (alarmMgr.getRadioUrl().isEmpty()) {
        Serial.println("[ALARM] No station stored; alarm stays silent.");
        return;
    }
    setRadioVolume(alarmMgr.getVolume(), false);
    currentTitle = STATUS_ALARM_TRIGGERED;
    isPlaying = true;
    currentStation = alarmMgr.getRadioName();
    currentStreamUrl = alarmMgr.getRadioUrl();
    Serial.printf("[ALARM] Starting alarm playback: %s (%s) at volume %d\n", 
                  currentStation.c_str(), currentStreamUrl.c_str(), currentVolume);
    audio.connecttohost(currentStreamUrl.c_str());
    alarmPlayback = true;
}

// Stops a running alarm (test) without touching the persisted power state and restores the stored radio volume
void stopAlarmPlayback() {
    Serial.println("[ALARM] Stopping alarm playback.");
    clearSdPlayAll();
    audio.stopSong();
    isPlaying = false;
    alarmPlayback = false;
    currentTitle = "--";
    currentBitrate = "--";
    loadPersistedVolume();
    audio.setVolume(currentVolume);
}

// Web server route: save alarm settings
void handleAlarmSave() {
    if (alarmMgr.getSource() != "sd" && stationMgr.size() == 0) {
        server.send(409, "text/plain", NO_M3U_STATIONS_SAVED);
        return;
    }
    if (server.hasArg("hour") && server.hasArg("minute")) {
        int hour = server.arg("hour").toInt();
        int minute = server.arg("minute").toInt();
        bool enabled = (server.arg("enabled") == "1" || server.arg("enabled") == "true");
        int volume = server.hasArg("volume") ? server.arg("volume").toInt() : 15;
        alarmMgr.saveSettings(hour, minute, enabled, volume);
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", INVALID_PARAMETERS);
}

void handleRadioAlarmSave() {
    StoredStation station;
    if (!stationMgr.getLastSelected(station)) {
        server.send(409, "text/plain", NO_LAST_SELECTED_STATION);
        return;
    }
    alarmMgr.saveRadioSource(station.name, station.url);
    server.send(200, "text/plain", "OK");
}

// Web server route: test alarm now (a second call while the alarm is playing stops it)
void handleAlarmTest() {
    if (alarmPlaybackActive()) {
        stopAlarmPlayback();
        server.send(200, "text/plain", "OK");
        return;
    }
    if (alarmMgr.getSource() == "sd" && (!sdMgr.isMounted() || !sdMgr.exists(alarmMgr.getSdPath()))) {
        server.send(409, "text/plain", ALARM_SD_FILE_NOT_AVAILABLE);
        return;
    }
    if (alarmMgr.getSource() != "sd" && stationMgr.size() == 0) {
        server.send(409, "text/plain", NO_M3U_STATIONS_SAVED);
        return;
    }
    triggerAlarmPlayback();
    server.send(200, "text/plain", "OK");
}

// Web server route: live alarm volume preview; applies only while an alarm is playing and is not persisted
void handleAlarmVolume() {
    if (!server.hasArg("val")) {
        server.send(400, "text/plain", INVALID_VALUE);
        return;
    }
    if (alarmPlaybackActive()) setRadioVolume(server.arg("val").toInt(), false);
    server.send(200, "text/plain", "OK");
}

// Web server route: status API
void handleStatus() {
    String json = "{";
    json += "\"playing\":" + String(audio.isRunning() ? "true" : "false") + ",";
    json += "\"station\":\"" + StationManager::jsonEscape(currentStation) + "\",";
    json += "\"title\":\"" + StationManager::jsonEscape(currentTitle) + "\",";
    json += "\"bitrate_raw\":\"" + StationManager::jsonEscape(currentBitrate) + "\",";
    json += "\"volume\":" + String(currentVolume) + ",";
    json += "\"announce_volume\":" + String(currentAnnounceVolume) + ",";
    json += "\"announcing\":" + String(isAnnouncing ? "true" : "false") + ",";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"free_psram\":" + String(ESP.getFreePsram()) + ",";
    json += "\"ap_mode\":" + String(wifiMgr.isApMode() ? "true" : "false") + ",";
    json += "\"ip\":\"" + (wifiMgr.isApMode() ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\",";
    json += "\"ssid\":\"" + StationManager::jsonEscape(!wifiMgr.isApMode() && WiFi.status() == WL_CONNECTED ? WiFi.SSID() : String("")) + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleStations() {
    long offset = server.hasArg("offset") ? server.arg("offset").toInt() : 0;
    long limit = server.hasArg("limit") ? server.arg("limit").toInt() : 20;
    offset = max(0L, offset);
    limit = constrain(limit, 1L, 100L);
    server.send(200, "application/json", stationMgr.search(server.arg("q"), (size_t)offset, (size_t)limit));
}

void handleStationDelete() {
    if (server.hasArg("id") && stationMgr.remove(server.arg("id").toInt())) {
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", INVALID_STATION_ID);
}

void handleStationsClear() {
    if (!stationMgr.clear()) {
        server.send(500, "text/plain", STATION_LIST_CLEAR_FAILED);
        return;
    }
    server.send(200, "text/plain", "OK");
}

void handleFavorites() {
    server.send(200, "application/json", stationMgr.favJson());
}

void handleFavoriteAdd() {
    StoredStation station;
    if (!server.hasArg("id") || !stationMgr.get(server.arg("id").toInt(), station)) {
        server.send(400, "text/plain", INVALID_STATION_ID);
        return;
    }
    if (!stationMgr.favAdd(station)) {
        server.send(409, "text/plain", String(FAVORITES_LIMIT_PREFIX) + String((unsigned)StationManager::MAX_FAVORITES) + FAVORITES_LIMIT_SUFFIX);
        return;
    }
    mqttMgr.refreshDiscovery();
    server.send(200, "text/plain", "OK");
}

void handleFavoriteRemove() {
    if (!server.hasArg("idx") || !stationMgr.favRemove(server.arg("idx").toInt())) {
        server.send(400, "text/plain", INVALID_FAVORITE);
        return;
    }
    mqttMgr.refreshDiscovery();
    server.send(200, "text/plain", "OK");
}

void handleFavoritePlay() {
    StoredStation station;
    if (!server.hasArg("idx") || !stationMgr.favGet(server.arg("idx").toInt(), station)) {
        server.send(400, "text/plain", INVALID_FAVORITE);
        return;
    }
    playStoredStation(station);
    server.send(200, "text/plain", "OK");
}

void handleSdFiles() {
    server.send(200, "application/json", sdMgr.filesJson());
}

bool sdFileProtected(const String &path) {
    if ((audio.isRunning() && currentStreamUrl.equalsIgnoreCase(path)) ||
        (isAnnouncing && previousState.url.equalsIgnoreCase(path)) ||
        (alarmMgr.getSource() == "sd" && alarmMgr.getSdPath().equalsIgnoreCase(path))) return true;
    for (const String &queued : sdPlayAllPaths) {
        if (queued.equalsIgnoreCase(path)) return true;
    }
    return false;
}

void handleSdDownload() {
    const String path = server.arg("path");
    if (!sdMgr.isMounted() || !SdManager::isUserAudioPath(path)) {
        server.send(400, "text/plain", INVALID_AUDIO_PATH_OR_NO_SD);
        return;
    }
    File file = sdMgr.filesystem().open(path, "r");
    if (!file || file.isDirectory()) {
        server.send(404, "text/plain", FILE_NOT_FOUND);
        return;
    }
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + path.substring(path.lastIndexOf('/') + 1) + "\"");
    server.streamFile(file, "application/octet-stream");
    file.close();
}

void handleSdDelete() {
    const String path = server.arg("path");
    if (!sdMgr.isMounted() || !SdManager::isUserAudioPath(path)) {
        server.send(400, "text/plain", INVALID_AUDIO_PATH_OR_NO_SD);
        return;
    }
    if (sdFileProtected(path)) {
        server.send(409, "text/plain", SD_FILE_IN_USE);
        return;
    }
    File file = sdMgr.filesystem().open(path, "r");
    if (!file || file.isDirectory()) {
        server.send(404, "text/plain", FILE_NOT_FOUND);
        return;
    }
    file.close();
    const bool removed = sdMgr.filesystem().remove(path);
    if (removed) mqttMgr.refreshDiscovery();
    server.send(removed ? 200 : 500, "text/plain", removed ? "OK" : SD_FILE_DELETE_FAILED);
}

File sdAudioUpload;
String sdAudioUploadPath;
String sdAudioUploadError;
size_t sdAudioUploadBytes = 0;
bool sdAudioUploadComplete = false;
const char *SD_AUDIO_TEMP = "/alarm/.upload.part";
const size_t SD_AUDIO_MAX_BYTES = 20 * 1024 * 1024;

void abortSdAudioUpload(const String &error) {
    if (sdAudioUpload) sdAudioUpload.close();
    if (!sdAudioUploadPath.isEmpty() && sdMgr.isMounted()) sdMgr.filesystem().remove(SD_AUDIO_TEMP);
    sdAudioUploadError = error;
    sdAudioUploadComplete = false;
}

void handleSdAudioUpload() {
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        if (!sdAudioUploadPath.isEmpty() || !sdAudioUploadError.isEmpty()) {
            abortSdAudioUpload(SD_UPLOAD_ONE_FILE_ONLY);
            return;
        }
        sdAudioUploadBytes = 0;
        sdAudioUploadComplete = false;
        if (!sdMgr.isMounted()) {
            abortSdAudioUpload(NO_SD_CARD);
            return;
        }
        const String name = upload.filename;
        if (name.isEmpty() || name.length() > 100 || name.startsWith(".") || name.indexOf('/') >= 0 || name.indexOf('\\') >= 0 ||
            !SdManager::isUserAudioPath("/alarm/" + name)) {
            abortSdAudioUpload(SD_FILE_INVALID);
            return;
        }
        sdAudioUploadPath = "/alarm/" + name;
        if (sdMgr.exists(sdAudioUploadPath)) {
            abortSdAudioUpload(SD_FILE_ALREADY_EXISTS);
            return;
        }
        if (!sdMgr.filesystem().exists("/alarm") && !sdMgr.filesystem().mkdir("/alarm")) {
            abortSdAudioUpload(SD_ALARM_DIR_CREATE_FAILED);
            return;
        }
        sdMgr.filesystem().remove(SD_AUDIO_TEMP);
        sdAudioUpload = sdMgr.filesystem().open(SD_AUDIO_TEMP, "w");
        if (!sdAudioUpload) abortSdAudioUpload(SD_UPLOAD_TEMP_CREATE_FAILED);
    } else if (upload.status == UPLOAD_FILE_WRITE && sdAudioUploadError.isEmpty()) {
        if (upload.currentSize > SD_AUDIO_MAX_BYTES - sdAudioUploadBytes) {
            abortSdAudioUpload(SD_FILE_TOO_LARGE);
        } else if (!sdAudioUpload || sdAudioUpload.write(upload.buf, upload.currentSize) != upload.currentSize) {
            abortSdAudioUpload(SD_FILE_WRITE_FAILED);
        } else {
            sdAudioUploadBytes += upload.currentSize;
        }
    } else if (upload.status == UPLOAD_FILE_END && sdAudioUploadError.isEmpty()) {
        if (sdAudioUpload) sdAudioUpload.close();
        if (!sdAudioUploadBytes || sdAudioUploadBytes != upload.totalSize) abortSdAudioUpload(SD_FILE_UPLOAD_INCOMPLETE);
        else sdAudioUploadComplete = true;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        abortSdAudioUpload(SD_FILE_UPLOAD_ABORTED);
        sdAudioUploadPath = "";
        sdAudioUploadError = "";
    }
}

void handleSdAudioUploadResult() {
    if (sdAudioUploadComplete && sdAudioUploadError.isEmpty()) {
        if (sdMgr.exists(sdAudioUploadPath) || !sdMgr.filesystem().rename(SD_AUDIO_TEMP, sdAudioUploadPath)) {
            abortSdAudioUpload(SD_FILE_SAVE_FAILED);
        }
    } else if (sdAudioUploadError.isEmpty()) {
        abortSdAudioUpload(SD_UPLOAD_NO_COMPLETE_FILE);
    }
    const bool success = sdAudioUploadComplete && sdAudioUploadError.isEmpty();
    server.send(success ? 200 : 400, "text/plain", success ? String(SD_FILE_SAVED_PREFIX) + sdAudioUploadPath : sdAudioUploadError);
    sdAudioUploadPath = "";
    sdAudioUploadError = "";
    sdAudioUploadComplete = false;
    if (success) mqttMgr.refreshDiscovery();
}

void handleSdPlay() {
    if (!server.hasArg("path") || !sdMgr.exists(server.arg("path"))) {
        server.send(404, "text/plain", SD_FILE_NOT_FOUND);
        return;
    }
    clearSdPlayAll();
    String path = server.arg("path");
    isPlaying = startSdPath(path);
    server.send(isPlaying ? 200 : 500, "text/plain", isPlaying ? "OK" : SD_PLAYBACK_FAILED);
}

void handleSdAlarm() {
    if (!server.hasArg("path") || !sdMgr.isMounted()) {
        server.send(409, "text/plain", SD_NOT_AVAILABLE);
        return;
    }
    String path = server.arg("path");
    if (!sdMgr.exists(path)) {
        server.send(404, "text/plain", SD_FILE_NOT_FOUND);
        return;
    }
    alarmMgr.saveSource("sd", path);
    server.send(200, "text/plain", "OK");
}

void handleSdPlayAll() {
    if (!sdMgr.isMounted()) {
        server.send(409, "text/plain", SD_NOT_AVAILABLE);
        return;
    }
    sdPlayAllPaths = sdMgr.paths();
    if (sdPlayAllPaths.empty()) {
        clearSdPlayAll();
        server.send(404, "text/plain", NO_PLAYABLE_SD_FILES);
        return;
    }
    sdPlayAllIndex = 0;
    sdPlayAllActive = true;
    isPlaying = startSdPath(sdPlayAllPaths[sdPlayAllIndex]);
    server.send(isPlaying ? 200 : 500, "text/plain", isPlaying ? "OK" : SD_PLAYBACK_FAILED);
}

void handleSdStopAll() {
    clearSdPlayAll();
    audio.stopSong();
    isPlaying = false;
    savePersistedRadioPower(false);
    server.send(200, "text/plain", "OK");
}

bool m3uUploadDone = false;
bool m3uUploadOk = false;
size_t m3uImportedCount = 0;

String m3uResultJson(size_t imported) {
    return "{\"imported\":" + String((unsigned)imported) + ",\"total\":" + String((unsigned)stationMgr.size()) +
           ",\"storage\":\"" + (stationMgr.usesSd() ? "sd" : "nvs") + "\"}";
}

// Mode via URL parameter (?mode=replace|append), because form fields are not parsed yet when the upload starts
void handleM3uImport() {
    if (m3uUploadDone) {
        m3uUploadDone = false;
        if (!m3uUploadOk) {
            server.send(500, "text/plain", STATION_LIST_WRITE_FAILED);
            return;
        }
        server.send(200, "application/json", m3uResultJson(m3uImportedCount));
        return;
    }
    String body = server.arg("plain");
    if (body.isEmpty()) {
        server.send(400, "text/plain", M3U_BODY_MISSING);
        return;
    }
    if (!stationMgr.beginImport(server.arg("mode") == "replace")) {
        server.send(500, "text/plain", STATION_LIST_WRITE_FAILED);
        return;
    }
    stationMgr.feed((const uint8_t *)body.c_str(), body.length());
    server.send(200, "application/json", m3uResultJson(stationMgr.endImport()));
}

void handleM3uUpload() {
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        m3uUploadDone = false;
        m3uImportedCount = 0;
        m3uUploadOk = stationMgr.beginImport(server.arg("mode") == "replace");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        stationMgr.feed(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        m3uImportedCount = stationMgr.endImport();
        m3uUploadDone = true;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        stationMgr.abortImport();
        m3uUploadOk = false;
    }
}

// Web server route: play a station from the list
void handleStation() {
    if (server.hasArg("id")) {
        StoredStation station;
        if (stationMgr.get(server.arg("id").toInt(), station)) {
            playStoredStation(station);
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(400, "text/plain", INVALID_STATION_ID);
}

// Web server route: play a custom URL
void handlePlay() {
    if (server.hasArg("url")) {
        clearSdPlayAll();
        alarmPlayback = false;
        currentStreamUrl = server.arg("url");
        currentStation = STATUS_CUSTOM_STREAM;
        currentTitle = STATUS_CONNECTING;
        currentBitrate = "--";
        isPlaying = true;
        savePersistedRadioPower(true);
        Serial.printf("[RADIO] Starting custom URL: %s\n", currentStreamUrl.c_str());
        audio.connecttohost(currentStreamUrl.c_str());
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", NO_URL_GIVEN);
}

// Web server route: announcement / TTS with automatic resume
void handleAnnounce() {
    if (server.hasArg("url")) {
        int vol = server.hasArg("volume") ? server.arg("volume").toInt() : (server.hasArg("vol") ? server.arg("vol").toInt() : -1);
        playAnnouncement(server.arg("url"), vol);
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", NO_ANNOUNCE_URL_GIVEN);
}

// Web server route: set announcement volume
void handleAnnounceVolume() {
    if (server.hasArg("val")) {
        setAnnounceVolume(server.arg("val").toInt());
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", INVALID_VALUE);
}

void stopPlayback() {
    Serial.println("[RADIO] Stop");
    clearSdPlayAll();
    alarmPlayback = false;
    audio.stopSong();
    isPlaying = false;
    savePersistedRadioPower(false);
    currentTitle = "--";
    currentBitrate = "--";
}

// Web server route: stop
void handleStop() {
    stopPlayback();
    server.send(200, "text/plain", "OK");
}

void handleResume() {
    if (currentStreamUrl.startsWith("http://") || currentStreamUrl.startsWith("https://")) {
        startStream(currentStation, currentStreamUrl);
    } else if (!startLastOrFirst()) {
        server.send(409, "text/plain", NO_STATION_TO_RESUME);
        return;
    }
    server.send(200, "text/plain", "OK");
}

// Web server route: volume
void handleVolume() {
    if (server.hasArg("val")) {
        int val = server.arg("val").toInt();
        setRadioVolume(val);
        Serial.printf("[RADIO] Volume set to: %d\n", currentVolume);
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(400, "text/plain", INVALID_VALUE);
}

// Web server route: scan Wi-Fi networks
void handleWifiScan() {
    server.send(200, "application/json", wifiMgr.scanNetworksJson());
}

// Web server route: save Wi-Fi credentials
void handleWifiSave() {
    if (server.hasArg("ssid")) {
        String ssid = server.arg("ssid");
        String pass = server.hasArg("pass") ? server.arg("pass") : "";
        wifiMgr.saveCredentials(ssid, pass);
        server.send(200, "text/plain", "OK");
        shouldReboot = true;
        rebootTimer = millis();
        return;
    }
    server.send(400, "text/plain", MISSING_PARAMETERS);
}

// Web server route: delete Wi-Fi credentials
void handleWifiReset() {
    wifiMgr.clearCredentials();
    server.send(200, "text/plain", "OK");
    shouldReboot = true;
    rebootTimer = millis();
}

void handleMqttConfig() {
    if (!server.hasArg("host")) {
        server.send(400, "text/plain", MQTT_HOST_MISSING);
        return;
    }
    mqttMgr.saveConfig(server.arg("host"), server.hasArg("port") ? server.arg("port").toInt() : 1883,
                       server.hasArg("user") ? server.arg("user") : "",
                       server.hasArg("pass") ? server.arg("pass") : "",
                       server.hasArg("base") ? server.arg("base") : "esp32radio");
    server.send(200, "text/plain", "OK");
}

void handleMqttConfigGet() {
    server.send(200, "application/json", mqttMgr.getConfigJson());
}

void handleMqttStatus() {
    server.send(200, "application/json", mqttMgr.getStatusJson());
}

void setup() {
    // Without a connected terminal the native USB serial keeps only 256 bytes; a larger buffer preserves the
    // boot log until the monitor reconnects after the reset (must be called before begin())
    Serial.setTxBufferSize(8192);
    Serial.begin(115200);
    // Wait briefly until the native USB-CDC port is connected
    unsigned long startMs = millis();
    while (!Serial && (millis() - startMs < 2500)) {
        delay(10);
    }
    delay(500);

    Serial.println("\n==========================================");
    Serial.println("  ESP32-S3 Internet Radio & MAX98357A");
    Serial.println("==========================================");

    // PSRAM check
    if (psramFound()) {
        Serial.printf("[SYS] PSRAM found! Total: %d KB, free: %d KB\n", 
            ESP.getPsramSize() / 1024, ESP.getFreePsram() / 1024);
    } else {
        Serial.println("[SYS] WARNING: No PSRAM found or PSRAM not enabled!");
    }
    Serial.printf("[SYS] Free heap: %d KB\n", ESP.getFreeHeap() / 1024);

    // I2S audio configuration
    Serial.printf("[I2S] Pins: BCLK=%d, LRC=%d, DOUT=%d\n", I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    loadPersistedVolume();
    audio.setVolume(currentVolume);
    Serial.printf("[AUDIO] Stored volume: %d\n", currentVolume);

    // Wi-Fi initialization (NVS -> STA -> AP fallback)
    sdMgr.begin();
    wifiMgr.initWifi(sdMgr.isMounted() ? &sdMgr.filesystem() : nullptr);
    stationMgr.begin(sdMgr.isMounted() ? &sdMgr.filesystem() : nullptr);
    Serial.printf("[RADIO] %u stations (%s), %u favorites\n", (unsigned)stationMgr.size(),
                  stationMgr.usesSd() ? "SD" : "NVS", (unsigned)stationMgr.favSize());
    mqttMgr.setStationListProvider(buildStationOptionsJson);
    mqttMgr.setSdListProvider(buildSdOptionsJson);
    mqttMgr.begin(handleMqttCommand);

    // Start alarm manager and NTP time sync
    alarmMgr.begin();

    // Register web server endpoints
    server.on("/", HTTP_GET, handleRoot);
    server.on("/setup", HTTP_GET, handleSetup);
    server.on("/alarm", HTTP_GET, handleAlarmPage);
    server.on("/sdcard", HTTP_GET, handleSdCard);   
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/station", HTTP_GET, handleStation);
    server.on("/api/play", HTTP_GET, handlePlay);
    server.on("/api/announce", HTTP_GET, handleAnnounce);
    server.on("/api/announce", HTTP_POST, handleAnnounce);
    server.on("/api/announce/volume", HTTP_GET, handleAnnounceVolume);
    server.on("/api/announce/volume", HTTP_POST, handleAnnounceVolume);
    server.on("/api/stop", HTTP_GET, handleStop);
    server.on("/api/resume", HTTP_GET, handleResume);
    server.on("/api/volume", HTTP_GET, handleVolume);
    server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
    server.on("/api/wifi/save", HTTP_POST, handleWifiSave);
    server.on("/api/wifi/reset", HTTP_POST, handleWifiReset);
    server.on("/api/stations", HTTP_GET, handleStations);
    server.on("/api/stations/delete", HTTP_POST, handleStationDelete);
    server.on("/api/stations/clear", HTTP_POST, handleStationsClear);
    server.on("/api/favorites", HTTP_GET, handleFavorites);
    server.on("/api/favorites/add", HTTP_POST, handleFavoriteAdd);
    server.on("/api/favorites/remove", HTTP_POST, handleFavoriteRemove);
    server.on("/api/favorite", HTTP_GET, handleFavoritePlay);
    server.on("/api/sd/files", HTTP_GET, handleSdFiles);
    server.on("/api/sd/download", HTTP_GET, handleSdDownload);
    server.on("/api/sd/delete", HTTP_POST, handleSdDelete);
    server.on("/api/sd/upload", HTTP_POST, handleSdAudioUploadResult, handleSdAudioUpload);
    server.on("/api/sd/play", HTTP_GET, handleSdPlay);
    server.on("/api/sd/play", HTTP_POST, handleSdPlay);
    server.on("/api/sd/alarm", HTTP_POST, handleSdAlarm);
    server.on("/api/sd/play-all", HTTP_POST, handleSdPlayAll);
    server.on("/api/sd/stop-all", HTTP_POST, handleSdStopAll);
    server.on("/api/stations/m3u", HTTP_POST, handleM3uImport, handleM3uUpload);
    server.on("/api/mqtt/config", HTTP_POST, handleMqttConfig);
    server.on("/api/mqtt/config", HTTP_GET, handleMqttConfigGet);
    server.on("/api/mqtt/status", HTTP_GET, handleMqttStatus);
    server.on("/api/alarm/status", HTTP_GET, handleAlarmStatus);
    server.on("/api/alarm/save", HTTP_POST, handleAlarmSave);
    server.on("/api/alarm/radio", HTTP_POST, handleRadioAlarmSave);
    server.on("/api/alarm/test", HTTP_POST, handleAlarmTest);
    server.on("/api/alarm/volume", HTTP_POST, handleAlarmVolume);
    server.on("/api/device/id", HTTP_GET, handleDeviceIdGet);
    server.onNotFound([]() { server.send(404, "text/plain", NOT_FOUND); });
    server.begin();
    Serial.println("[HTTP] Web server started on port 80.");

    // Auto-start the radio station in STA mode only
    if (radioPowerOn && !wifiMgr.isApMode() && WiFi.status() == WL_CONNECTED) {
        delay(1000);
        if (startLastOrFirst()) {
            Serial.printf("[RADIO] Auto-start: %s\n", currentStation.c_str());
        }
    }
}

void loop() {
    audio.loop();
    if (pendingResume) {
        pendingResume = false;
        resumeAfterAnnouncement();
    }
    if (pendingSdNext) {
        pendingSdNext = false;
        playNextSdFile();
    }
    server.handleClient();
    mqttMgr.loop();

    // Timeout guard for announcements in case the TTS stream breaks off
    if (isAnnouncing) {
        unsigned long now = millis();
        if (audio.isRunning()) announceLastActiveMs = now;
        if (now - announceLastActiveMs > ANNOUNCE_IDLE_TIMEOUT_MS || now - announceStartMs > ANNOUNCE_MAX_MS) {
            Serial.println("[ANNOUNCE] Timeout reached, returning to previous state.");
            resumeAfterAnnouncement();
        }
    }

    static unsigned long lastMqttState = 0;
    if (millis() - lastMqttState > 5000) {
        lastMqttState = millis();
        mqttMgr.publishState(audio.isRunning(), currentStation, currentTitle, currentVolume, currentAnnounceVolume, isAnnouncing);
        mqttMgr.publishAlarmState(alarmMgr.isEnabled(), alarmMgr.getHour(), alarmMgr.getMinute(), alarmMgr.getVolume(), alarmMgr.getSource(), alarmMgr.getSdPath());
        mqttMgr.publish("state/sd_current", currentStation.startsWith(SD_STATION_PREFIX) ? currentStreamUrl : "");
    }

    if (alarmMgr.checkAlarmTrigger()) {
        triggerAlarmPlayback();
    }

    if (shouldReboot && (millis() - rebootTimer > 1500)) {
        Serial.println("[SYS] Restarting system...");
        ESP.restart();
    }
}

// ==========================================
// Audio callbacks for metadata and status
// ==========================================
void audio_info(const char *info) {
    Serial.print("[AUDIO INFO] ");
    Serial.println(info);
}

void audio_showstation(const char *info) {
    Serial.print("[STATION] ");
    Serial.println(info);
    if (info && strlen(info) > 0) {
        currentStation = String(info);
    }
}

void audio_showstreamtitle(const char *info) {
    Serial.print("[STREAM TITLE] ");
    Serial.println(info);
    if (info && strlen(info) > 0) {
        currentTitle = String(info);
    }
}

void audio_bitrate(const char *info) {
    Serial.print("[BITRATE] ");
    Serial.println(info);
    if (info && strlen(info) > 0) {
        currentBitrate = String(info);
    }
}

void audio_eof_mp3(const char *info) {
    Serial.print("[EOF MP3] ");
    Serial.println(info);
    if (isAnnouncing) {
        pendingResume = true;
        return;
    }
    if (sdPlayAllActive) pendingSdNext = true;
}

void audio_eof_speech(const char *info) {
    Serial.print("[EOF SPEECH] ");
    Serial.println(info);
    if (isAnnouncing) pendingResume = true;
}
