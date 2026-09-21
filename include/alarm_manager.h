#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

class AlarmManager {
private:
    Preferences prefs;
    int _hour = 7;
    int _minute = 0;
    bool _enabled = false;
    int _volume = 15;
    String _source = "radio";
    String _sdPath;
    String _radioName;
    String _radioUrl;
    bool _triggeredThisMinute = false;

    static String jsonEscape(const String &value) {
        String result;
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '\\' || c == '"') result += '\\';
            result += c;
        }
        return result;
    }

public:
    void begin() {
        loadSettings();
        // CET-1CEST,M3.5.0,M10.5.0/3 = Mitteleuropaeische Zeit mit Sommer-/Winterzeit
        configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
        Serial.println("[ALARM] NTP Zeitsynchronisation gestartet (Mitteleuropa CET/CEST).");
    }

    void loadSettings() {
        prefs.begin("alarm_config", true);
        _hour = prefs.getInt("hour", 7);
        _minute = prefs.getInt("min", 0);
        _enabled = prefs.getBool("enabled", false);
        _volume = prefs.getInt("vol", 15);
        _source = prefs.getString("source", "radio");
        _sdPath = prefs.getString("sd_path", "");
        _radioName = prefs.getString("radio_name", "");
        _radioUrl = prefs.getString("radio_url", "");
        prefs.end();
    }

    void saveSettings(int hour, int minute, bool enabled, int volume) {
        _hour = hour;
        _minute = minute;
        _enabled = enabled;
        _volume = volume;

        prefs.begin("alarm_config", false);
        prefs.putInt("hour", _hour);
        prefs.putInt("min", _minute);
        prefs.putBool("enabled", _enabled);
        prefs.putInt("vol", _volume);
        prefs.end();

        Serial.printf("[ALARM] Einstellungen gespeichert: %02d:%02d, Aktiv: %s, Vol: %d\n",
                      _hour, _minute, _enabled ? "JA" : "NEIN", _volume);
    }

    void saveSource(const String &source, const String &sdPath) {
        _source = source == "sd" ? "sd" : "radio";
        _sdPath = _source == "sd" ? sdPath : "";
        prefs.begin("alarm_config", false);
        prefs.putString("source", _source);
        prefs.putString("sd_path", _sdPath);
        prefs.end();
    }

    // Einzel-Setter für granulare MQTT-Entities (ändern jeweils nur ein Feld, Rest bleibt erhalten)
    void setEnabled(bool enabled) {
        _enabled = enabled;
        prefs.begin("alarm_config", false);
        prefs.putBool("enabled", _enabled);
        prefs.end();
    }

    void setTime(int hour, int minute) {
        _hour = constrain(hour, 0, 23);
        _minute = constrain(minute, 0, 59);
        prefs.begin("alarm_config", false);
        prefs.putInt("hour", _hour);
        prefs.putInt("min", _minute);
        prefs.end();
    }

    void setVolume(int volume) {
        _volume = constrain(volume, 0, 21);
        prefs.begin("alarm_config", false);
        prefs.putInt("vol", _volume);
        prefs.end();
    }

    void setSourceType(const String &source) {
        _source = source == "sd" ? "sd" : "radio";
        prefs.begin("alarm_config", false);
        prefs.putString("source", _source);
        prefs.end();
    }

    void saveRadioSource(const String &name, const String &url) {
        _source = "radio";
        _sdPath = "";
        _radioName = name;
        _radioUrl = url;
        prefs.begin("alarm_config", false);
        prefs.putString("source", _source);
        prefs.putString("sd_path", _sdPath);
        prefs.putString("radio_name", _radioName);
        prefs.putString("radio_url", _radioUrl);
        prefs.end();
    }

    bool isTimeSynced() const {
        time_t now = time(nullptr);
        struct tm timeinfo;
        if (!localtime_r(&now, &timeinfo)) return false;
        return timeinfo.tm_year > 120; // Jahr > 2020 bedeutet NTP ist synchronisiert
    }

    String getCurrentTimeStr() const {
        time_t now = time(nullptr);
        struct tm timeinfo;
        if (!localtime_r(&now, &timeinfo) || timeinfo.tm_year <= 120) {
            return "Warte auf Uhrzeit (NTP)...";
        }
        char buf[32];
        strftime(buf, sizeof(buf), "%H:%02M:%02S Uhr", &timeinfo);
        return String(buf);
    }

    int getHour() const { return _hour; }
    int getMinute() const { return _minute; }
    bool isEnabled() const { return _enabled; }
    int getVolume() const { return _volume; }
    const String &getSource() const { return _source; }
    const String &getSdPath() const { return _sdPath; }
    const String &getRadioName() const { return _radioName; }
    const String &getRadioUrl() const { return _radioUrl; }

    // Prueft in loop(), ob Weckzeit erreicht ist
    bool checkAlarmTrigger() {
        if (!_enabled) {
            _triggeredThisMinute = false;
            return false;
        }

        time_t now = time(nullptr);
        struct tm timeinfo;
        if (!localtime_r(&now, &timeinfo) || timeinfo.tm_year <= 120) {
            return false;
        }

        if (timeinfo.tm_hour == _hour && timeinfo.tm_min == _minute) {
            if (!_triggeredThisMinute) {
                _triggeredThisMinute = true;
                Serial.printf("[ALARM] WECKER AUSSGELÖST um %02d:%02d Uhr!\n", _hour, _minute);
                return true;
            }
        } else {
            _triggeredThisMinute = false;
        }

        return false;
    }

    String getStatusJson() const {
        String json = "{";
        json += "\"current_time\":\"" + getCurrentTimeStr() + "\",";
        json += "\"time_synced\":" + String(isTimeSynced() ? "true" : "false") + ",";
        json += "\"hour\":" + String(_hour) + ",";
        json += "\"minute\":" + String(_minute) + ",";
        json += "\"enabled\":" + String(_enabled ? "true" : "false") + ",";
        json += "\"volume\":" + String(_volume) + ",";
        json += "\"source\":\"" + jsonEscape(_source) + "\",";
        json += "\"sd_path\":\"" + jsonEscape(_sdPath) + "\",";
        json += "\"radio_name\":\"" + jsonEscape(_radioName) + "\",";
        json += "\"radio_url\":\"" + jsonEscape(_radioUrl) + "\"";
        json += "}";
        return json;
    }
};

extern AlarmManager alarmMgr;
