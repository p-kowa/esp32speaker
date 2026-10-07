#pragma once
#include <Arduino.h>
#include <FS.h>
#include <Preferences.h>

struct StoredStation {
    String name;
    String url;
};

namespace station_files {
static const char DIR[] = "/radio";
// Eine Zeile pro Sender: "name<TAB>url\n"
static const char DATA[] = "/radio/stations.tsv";
// Pro Sender ein uint32 mit dem Byte-Offset der Zeile in DATA
static const char INDEX[] = "/radio/stations.idx";
// FATFS ist ohne LFN gebaut: nur 8.3-Dateinamen
static const char DATA_TMP[] = "/radio/newlist.tsv";
static const char INDEX_TMP[] = "/radio/newlist.idx";
}

class StationManager {
public:
    enum : size_t { NVS_MAX_STATIONS = 24, MAX_FAVORITES = 50, MAX_LINE = 1024 };

    static String jsonEscape(const String &value) {
        String escaped;
        escaped.reserve(value.length() + 8);
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            switch (c) {
                case '\\': escaped += "\\\\"; break;
                case '"': escaped += "\\\""; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    if ((uint8_t)c < 0x20) {
                        char buf[7];
                        snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)c);
                        escaped += buf;
                    } else {
                        escaped += c;
                    }
            }
        }
        return escaped;
    }

private:
    class LineReader {
    public:
        explicit LineReader(File &file) : file(file) {}

        // Ueberlange Zeilen werden auf MAX_LINE gekuerzt
        bool next(String &line) {
            line = "";
            bool any = false;
            while (true) {
                if (pos >= len) {
                    len = file.read(buf, sizeof(buf));
                    pos = 0;
                    if (len == 0) return any;
                }
                char c = (char)buf[pos++];
                any = true;
                if (c == '\n') return true;
                if (c != '\r' && line.length() < MAX_LINE) line += c;
            }
        }

    private:
        File &file;
        uint8_t buf[512];
        size_t len = 0;
        size_t pos = 0;
    };

    fs::FS *fs = nullptr;
    size_t sdCount = 0;
    StoredStation nvsStations[NVS_MAX_STATIONS];
    size_t nvsCount = 0;
    StoredStation favorites[MAX_FAVORITES];
    size_t favCount = 0;
    String lastName;
    String lastUrl;
    Preferences prefs;

    bool importing = false;
    bool importReplace = false;
    File importData;
    File importIndex;
    uint32_t importOffset = 0;
    String importLine;
    String pendingName;
    size_t importedCount = 0;
    size_t nvsPrevCount = 0;
    bool firstImportLine = true;

    bool sdMode() const { return fs != nullptr; }

    static bool parseLine(const String &line, StoredStation &station) {
        int tab = line.indexOf('\t');
        if (tab <= 0) return false;
        station.name = line.substring(0, tab);
        station.url = line.substring(tab + 1);
        return !station.url.isEmpty();
    }

    static void appendStationJson(String &json, size_t id, const StoredStation &station) {
        if (json.length() > 0) json += ',';
        json += "{\"id\":" + String(id) + ",\"name\":\"" + jsonEscape(station.name) +
                "\",\"url\":\"" + jsonEscape(station.url) + "\"}";
    }

    static bool writeEntry(File &data, File &index, uint32_t &offset, const String &name, const String &url) {
        String line = name + '\t' + url + '\n';
        if (data.print(line) != line.length()) return false;
        bool ok = index.write((const uint8_t *)&offset, sizeof(offset)) == sizeof(offset);
        offset += line.length();
        return ok;
    }

    void removeIfExists(const char *path) {
        if (fs->exists(path)) fs->remove(path);
    }

    void replaceFilesWithTmp() {
        removeIfExists(station_files::DATA);
        removeIfExists(station_files::INDEX);
        fs->rename(station_files::DATA_TMP, station_files::DATA);
        fs->rename(station_files::INDEX_TMP, station_files::INDEX);
    }

    void refreshSdCount() {
        sdCount = 0;
        if (!fs->exists(station_files::INDEX)) return;
        File index = fs->open(station_files::INDEX, FILE_READ);
        if (!index) return;
        sdCount = index.size() / sizeof(uint32_t);
        index.close();
    }

    void loadList(const char *ns, StoredStation *list, size_t maxCount, size_t &count) {
        prefs.begin(ns, false);
        count = prefs.getUInt("count", 0);
        if (count > maxCount) count = 0;
        for (size_t i = 0; i < count; ++i) {
            list[i].name = prefs.getString(("n" + String(i)).c_str(), "");
            list[i].url = prefs.getString(("u" + String(i)).c_str(), "");
        }
        prefs.end();
    }

    void persistList(const char *ns, const StoredStation *list, size_t count, size_t previousCount) {
        prefs.begin(ns, false);
        prefs.putUInt("count", count);
        for (size_t i = 0; i < count; ++i) {
            prefs.putString(("n" + String(i)).c_str(), list[i].name);
            prefs.putString(("u" + String(i)).c_str(), list[i].url);
        }
        for (size_t i = count; i < previousCount; ++i) {
            prefs.remove(("n" + String(i)).c_str());
            prefs.remove(("u" + String(i)).c_str());
        }
        prefs.end();
    }

    void loadNvsStations() {
        loadList("stations", nvsStations, NVS_MAX_STATIONS, nvsCount);
    }

    void addImportedEntry(const String &name, const String &url) {
        if (sdMode()) {
            if (writeEntry(importData, importIndex, importOffset, name, url)) ++importedCount;
            return;
        }
        if (nvsCount < NVS_MAX_STATIONS) {
            nvsStations[nvsCount++] = {name, url};
            ++importedCount;
        }
    }

    void processImportLine() {
        String line = importLine;
        if (firstImportLine) {
            firstImportLine = false;
            if (line.startsWith("\xEF\xBB\xBF")) line.remove(0, 3);
        }
        line.trim();
        if (line.isEmpty()) return;
        if (line.startsWith("#EXTINF:")) {
            int comma = line.indexOf(',');
            pendingName = comma >= 0 ? line.substring(comma + 1) : "";
            pendingName.trim();
            return;
        }
        if (line.startsWith("#")) return;
        if (line.indexOf("://") < 0) {
            pendingName = "";
            return;
        }
        String name = pendingName.isEmpty() ? line : pendingName;
        pendingName = "";
        name.replace('\t', ' ');
        line.replace('\t', ' ');
        addImportedEntry(name, line);
    }

public:
    // sdFs == nullptr: Fallback auf NVS mit max. NVS_MAX_STATIONS Sendern
    void begin(fs::FS *sdFs) {
        fs = sdFs;
        loadNvsStations();
        loadList("favorites", favorites, MAX_FAVORITES, favCount);
        prefs.begin("stations", false);
        lastName = prefs.getString("last_name", "");
        lastUrl = prefs.getString("last_url", "");
        prefs.end();

        if (!sdMode()) return;
        if (!fs->exists(station_files::DIR)) fs->mkdir(station_files::DIR);
        refreshSdCount();

        // Einmalige Uebernahme der alten NVS-Liste auf die SD-Karte
        if (sdCount == 0 && nvsCount > 0 && beginImport(false)) {
            for (size_t i = 0; i < nvsCount; ++i) addImportedEntry(nvsStations[i].name, nvsStations[i].url);
            endImport();
        }
    }

    bool usesSd() const { return sdMode(); }

    size_t size() const { return sdMode() ? sdCount : nvsCount; }

    bool get(size_t index, StoredStation &station) {
        if (!sdMode()) {
            if (index >= nvsCount) return false;
            station = nvsStations[index];
            return true;
        }
        if (index >= sdCount) return false;
        File idx = fs->open(station_files::INDEX, FILE_READ);
        if (!idx) return false;
        uint32_t offset = 0;
        bool ok = idx.seek(index * sizeof(uint32_t)) && idx.read((uint8_t *)&offset, sizeof(offset)) == sizeof(offset);
        idx.close();
        if (!ok) return false;

        File data = fs->open(station_files::DATA, FILE_READ);
        if (!data) return false;
        String line;
        if (data.seek(offset)) {
            LineReader reader(data);
            ok = reader.next(line);
        } else {
            ok = false;
        }
        data.close();
        return ok && parseLine(line, station);
    }

    // JSON: {"total":N,"offset":O,"storage":"sd|nvs","stations":[{id,name,url}]}
    String search(const String &query, size_t offset, size_t limit) {
        String q = query;
        q.trim();
        q.toLowerCase();
        String items;
        size_t total = 0;

        auto consider = [&](size_t id, const StoredStation &station) {
            if (!q.isEmpty()) {
                String name = station.name;
                name.toLowerCase();
                if (name.indexOf(q) < 0) return;
            }
            if (total >= offset && total < offset + limit) appendStationJson(items, id, station);
            ++total;
        };

        if (!sdMode()) {
            for (size_t i = 0; i < nvsCount; ++i) consider(i, nvsStations[i]);
        } else if (q.isEmpty()) {
            total = sdCount;
            if (offset < sdCount) {
                File data = fs->open(station_files::DATA, FILE_READ);
                File idx = fs->open(station_files::INDEX, FILE_READ);
                uint32_t start = 0;
                if (data && idx && idx.seek(offset * sizeof(uint32_t)) &&
                    idx.read((uint8_t *)&start, sizeof(start)) == sizeof(start) && data.seek(start)) {
                    LineReader reader(data);
                    String line;
                    StoredStation station;
                    for (size_t id = offset; id < sdCount && id < offset + limit && reader.next(line); ++id) {
                        if (parseLine(line, station)) appendStationJson(items, id, station);
                    }
                }
                data.close();
                idx.close();
            }
        } else if (fs->exists(station_files::DATA)) {
            File data = fs->open(station_files::DATA, FILE_READ);
            if (data) {
                LineReader reader(data);
                String line;
                StoredStation station;
                size_t id = 0;
                while (reader.next(line)) {
                    if (parseLine(line, station)) consider(id, station);
                    // Idle-Task laufen lassen (Task-Watchdog) bei grossen Listen
                    if ((++id & 0xFF) == 0) vTaskDelay(1);
                }
                data.close();
            }
        }

        return "{\"total\":" + String(total) + ",\"offset\":" + String(offset) + ",\"storage\":\"" +
               (sdMode() ? "sd" : "nvs") + "\",\"stations\":[" + items + "]}";
    }

    bool remove(size_t index) {
        if (importing) return false;
        if (!sdMode()) {
            if (index >= nvsCount) return false;
            size_t previous = nvsCount;
            for (size_t i = index; i + 1 < nvsCount; ++i) nvsStations[i] = nvsStations[i + 1];
            --nvsCount;
            persistList("stations", nvsStations, nvsCount, previous);
            return true;
        }
        if (index >= sdCount) return false;
        File src = fs->open(station_files::DATA, FILE_READ);
        File dst = fs->open(station_files::DATA_TMP, FILE_WRITE);
        File dstIdx = fs->open(station_files::INDEX_TMP, FILE_WRITE);
        if (!src || !dst || !dstIdx) {
            src.close();
            dst.close();
            dstIdx.close();
            return false;
        }
        LineReader reader(src);
        String line;
        StoredStation station;
        uint32_t offset = 0;
        size_t id = 0;
        while (reader.next(line)) {
            if (id != index && parseLine(line, station)) writeEntry(dst, dstIdx, offset, station.name, station.url);
            if ((++id & 0xFF) == 0) vTaskDelay(1);
        }
        src.close();
        dst.close();
        dstIdx.close();
        replaceFilesWithTmp();
        refreshSdCount();
        return true;
    }

    bool clear() {
        if (importing) return false;
        if (!sdMode()) {
            size_t previous = nvsCount;
            nvsCount = 0;
            persistList("stations", nvsStations, 0, previous);
            return true;
        }
        removeIfExists(station_files::DATA);
        removeIfExists(station_files::INDEX);
        refreshSdCount();
        return sdCount == 0;
    }

    // ---------- Streaming-Import (M3U) ----------
    bool beginImport(bool replace) {
        if (importing) abortImport();
        importReplace = replace;
        importLine = "";
        importLine.reserve(256);
        pendingName = "";
        importedCount = 0;
        firstImportLine = true;
        nvsPrevCount = nvsCount;

        if (sdMode()) {
            importData = fs->open(replace ? station_files::DATA_TMP : station_files::DATA, replace ? FILE_WRITE : FILE_APPEND);
            importIndex = fs->open(replace ? station_files::INDEX_TMP : station_files::INDEX, replace ? FILE_WRITE : FILE_APPEND);
            if (!importData || !importIndex) {
                importData.close();
                importIndex.close();
                return false;
            }
            importOffset = replace ? 0 : importData.size();
        } else if (replace) {
            nvsCount = 0;
        }
        importing = true;
        return true;
    }

    void feed(const uint8_t *data, size_t length) {
        if (!importing) return;
        for (size_t i = 0; i < length; ++i) {
            char c = (char)data[i];
            if (c == '\n') {
                processImportLine();
                importLine = "";
            } else if (c != '\r' && importLine.length() < MAX_LINE) {
                importLine += c;
            }
        }
    }

    size_t endImport() {
        if (!importing) return 0;
        if (!importLine.isEmpty()) {
            processImportLine();
            importLine = "";
        }
        importing = false;
        if (sdMode()) {
            importData.close();
            importIndex.close();
            if (importReplace) replaceFilesWithTmp();
            refreshSdCount();
        } else {
            persistList("stations", nvsStations, nvsCount, nvsPrevCount);
        }
        return importedCount;
    }

    void abortImport() {
        if (!importing) return;
        importing = false;
        if (sdMode()) {
            importData.close();
            importIndex.close();
            if (importReplace) {
                removeIfExists(station_files::DATA_TMP);
                removeIfExists(station_files::INDEX_TMP);
            }
            refreshSdCount();
        } else {
            loadNvsStations();
        }
    }

    size_t importM3u(const String &m3u) {
        if (!beginImport(false)) return 0;
        feed((const uint8_t *)m3u.c_str(), m3u.length());
        return endImport();
    }

    // ---------- Zuletzt gewaehlter Sender ----------
    void setLastSelected(const StoredStation &station) {
        if (station.name == lastName && station.url == lastUrl) return;
        lastName = station.name;
        lastUrl = station.url;
        prefs.begin("stations", false);
        prefs.putString("last_name", lastName);
        prefs.putString("last_url", lastUrl);
        prefs.end();
    }

    bool getLastSelected(StoredStation &station) const {
        if (lastUrl.isEmpty()) return false;
        station = {lastName, lastUrl};
        return true;
    }

    // ---------- Favoriten ----------
    size_t favSize() const { return favCount; }

    bool favGet(size_t index, StoredStation &station) const {
        if (index >= favCount) return false;
        station = favorites[index];
        return true;
    }

    bool favAdd(const StoredStation &station) {
        for (size_t i = 0; i < favCount; ++i) {
            if (favorites[i].url == station.url) return true;
        }
        if (favCount >= MAX_FAVORITES) return false;
        favorites[favCount++] = station;
        persistList("favorites", favorites, favCount, favCount - 1);
        return true;
    }

    bool favRemove(size_t index) {
        if (index >= favCount) return false;
        size_t previous = favCount;
        for (size_t i = index; i + 1 < favCount; ++i) favorites[i] = favorites[i + 1];
        --favCount;
        persistList("favorites", favorites, favCount, previous);
        return true;
    }

    String favJson() const {
        String items;
        for (size_t i = 0; i < favCount; ++i) {
            if (i > 0) items += ',';
            items += "{\"idx\":" + String(i) + ",\"name\":\"" + jsonEscape(favorites[i].name) +
                     "\",\"url\":\"" + jsonEscape(favorites[i].url) + "\"}";
        }
        return "{\"favorites\":[" + items + "]}";
    }

    String favNamesJson() const {
        String json = "[";
        for (size_t i = 0; i < favCount; ++i) {
            if (i > 0) json += ',';
            json += "\"" + jsonEscape(favorites[i].name) + "\"";
        }
        json += ']';
        return json;
    }
};

extern StationManager stationMgr;
