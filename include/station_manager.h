#pragma once
#include <Arduino.h>
#include <Preferences.h>

struct StoredStation {
    String name;
    String url;
};

class StationManager {
private:
    static constexpr size_t MAX_STATIONS = 24;
    StoredStation stations[MAX_STATIONS];
    size_t count = 0;
    int lastStation = -1;
    Preferences prefs;

    static String jsonEscape(const String &value) {
        String escaped;
        escaped.reserve(value.length() + 8);
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '\\' || c == '"') escaped += '\\';
            if (c == '\n') { escaped += "\\n"; continue; }
            if (c == '\r') { escaped += "\\r"; continue; }
            escaped += c;
        }
        return escaped;
    }

    void persist() {
        prefs.begin("stations", false);
        prefs.clear();
        prefs.putUInt("version", 1);
        prefs.putUInt("count", count);
        prefs.putInt("last", lastStation);
        for (size_t i = 0; i < count; ++i) {
            prefs.putString(("n" + String(i)).c_str(), stations[i].name);
            prefs.putString(("u" + String(i)).c_str(), stations[i].url);
        }
        prefs.end();
    }

public:
    void begin() {
        prefs.begin("stations", true);
        uint32_t version = prefs.getUInt("version", 0);
        size_t savedCount = prefs.getUInt("count", 0);
        lastStation = prefs.getInt("last", -1);
        prefs.end();

        if (version == 0) {
            clear();
            return;
        }

        if (savedCount > 0 && savedCount <= MAX_STATIONS) {
            prefs.begin("stations", true);
            count = savedCount;
            for (size_t i = 0; i < count; ++i) {
                stations[i].name = prefs.getString(("n" + String(i)).c_str(), "");
                stations[i].url = prefs.getString(("u" + String(i)).c_str(), "");
            }
            prefs.end();
            return;
        }

        count = 0;
    }

    bool add(const String &name, const String &url, bool save = true) {
        if (name.isEmpty() || url.isEmpty() || count >= MAX_STATIONS) return false;
        for (size_t i = 0; i < count; ++i) {
            if (stations[i].url == url) return false;
        }
        stations[count++] = {name, url};
        if (save) persist();
        return true;
    }

    void clear() {
        count = 0;
        lastStation = -1;
        persist();
    }

    bool get(size_t index, StoredStation &station) const {
        if (index >= count) return false;
        station = stations[index];
        return true;
    }

    bool remove(size_t index) {
        if (index >= count) return false;
        for (size_t i = index; i + 1 < count; ++i) {
            stations[i] = stations[i + 1];
        }
        --count;
        if (lastStation == (int)index) lastStation = -1;
        else if (lastStation > (int)index) --lastStation;
        persist();
        return true;
    }

    void setLastSelected(size_t index) {
        if (index < count) {
            lastStation = (int)index;
            persist();
        }
    }

    int getLastSelected() const { return lastStation; }

    size_t size() const { return count; }

    String json() const {
        String result = "[";
        for (size_t i = 0; i < count; ++i) {
            if (i > 0) result += ',';
            result += "{\"id\":" + String(i);
            result += ",\"name\":\"" + jsonEscape(stations[i].name) + "\"";
            result += ",\"url\":\"" + jsonEscape(stations[i].url) + "\"}";
        }
        result += ']';
        return result;
    }

    bool importM3u(const String &m3u) {
        String pendingName;
        bool changed = false;
        int start = 0;
        while (start < (int)m3u.length()) {
            int end = m3u.indexOf('\n', start);
            if (end < 0) end = m3u.length();
            String line = m3u.substring(start, end);
            line.trim();
            start = end + 1;

            if (line.startsWith("#EXTINF:")) {
                int comma = line.indexOf(',');
                pendingName = comma >= 0 ? line.substring(comma + 1) : "Internet Radio";
                pendingName.trim();
            } else if (!line.isEmpty() && !line.startsWith("#") && !pendingName.isEmpty()) {
                changed |= add(pendingName, line, false);
                pendingName = "";
            }
        }
        if (changed) persist();
        return changed;
    }
};

extern StationManager stationMgr;
