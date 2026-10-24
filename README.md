# ESP32 Speaker

Internet-Radio und SD-Audio-Player auf Basis eines ESP32-S3 mit Arduino/PlatformIO. Das Projekt kann ueber eine Weboberflaeche, MQTT und Home Assistant gesteuert werden.

## Funktionen

- Internet-Radio ueber gespeicherte M3U-Senderlisten
- Senderauswahl und Wiedergabe ueber MQTT
- Audioausgabe ueber MAX98357A-I2S-Verstaerker
- Wiedergabe von MP3-, WAV-, AAC- und FLAC-Dateien von einer Micro-SD-Karte
- SD-Dateien werden beim Start erkannt und als Home-Assistant-Auswahl angeboten
- Wecker mit:
  - Aktivierung und Deaktivierung
  - Uhrzeit
  - eigener Lautstaerke
  - Quelle Radio oder SD
  - auswaehlbarer SD-Datei
- MQTT Home Assistant Discovery
- WLAN-Konfiguration ueber Weboberflaeche
- Persistente Einstellungen im ESP32-NVS
- Eindeutige MQTT-Topics pro Geraet ueber die Chip-ID als Standard

## Hardware

| Funktion | ESP32-S3 GPIO |
| --- | ---: |
| I2S BCLK | 9 |
| I2S LRC / WS | 7 |
| I2S DOUT | 8 |
| SD SCK | 12 |
| SD MOSI | 11 |
| SD MISO | 13 |
| SD CS | 10 |

Die Pins koennen in `include/config.h` angepasst werden.

## Voraussetzungen

- PlatformIO in VS Code
- ESP32-S3 DevKitC-1
- MAX98357A-I2S-Verstaerker und Lautsprecher
- Micro-SD-Kartenleser fuer SPI
- WLAN
- Optional: MQTT-Broker, zum Beispiel Mosquitto
- Optional: Home Assistant mit aktivierter MQTT-Integration

## Bauen und Flashen

Projekt in VS Code oeffnen und anschliessend:

```text
PlatformIO: Build
PlatformIO: Upload
PlatformIO: Monitor
```

Oder in einer PlatformIO-Shell:

```powershell
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload
pio device monitor -b 115200
```

## Erste Einrichtung

1. Firmware flashen.
2. Mit dem vom ESP32 bereitgestellten WLAN-Access-Point verbinden, falls noch keine WLAN-Konfiguration gespeichert ist.
3. WLAN ueber die Weboberflaeche konfigurieren.
4. Unter `/setup` MQTT-Broker, Port, Benutzer, Passwort und optional ein eigenes Topic-Praefix eintragen.
5. Eine M3U-Senderliste ueber die Weboberflaeche importieren.
6. SD-Karte mit Audiodateien einlegen und das Geraet neu starten.

Die Weboberflaeche ist anschliessend unter der IP-Adresse des ESP32 erreichbar.

## Home Assistant

Home Assistant muss mit demselben MQTT-Broker verbunden sein. Die Firmware publiziert die Discovery-Konfiguration automatisch unter dem Standard-Prefix `homeassistant`.

Nach dem MQTT-Verbindungsaufbau wird ein Geraet mit dem Namen `ESP32 Radio (...)` angelegt. Unter diesem Geraet stehen unter anderem folgende Entities zur Verfuegung:

- Radio Power
- Radio Lautstaerke
- Radio Sender
- Radio Titel
- Radio Sender Auswahl
- SD Datei Auswahl
- Wecker Aktiv
- Weckzeit
- Wecker Lautstaerke
- Wecker Quelle
- Wecker SD-Datei, wenn abspielbare SD-Dateien vorhanden sind

Die Auswahl `SD Datei Auswahl` startet eine Datei sofort. Die Auswahl `Wecker SD-Datei` legt dagegen die Datei fuer den Wecker fest. Fuer den Wecker muss zusaetzlich `Wecker Quelle` auf `sd` stehen.

Die automatisch erzeugten Entities koennen direkt in Karten verwendet und dem passenden Home-Assistant-Bereich zugeordnet werden, zum Beispiel `Wohnkeller`.

## MQTT

### Home Assistant TTS audio

Home Assistant may serve `/api/tts_proxy/*.mp3` using HTTP chunked transfer
without a `Content-Length` header. ESP32-audioI2S 3.0.12 treats such a response
as an endless radio stream and reconnects after the final audio packet.

The build automatically applies [scripts/patch_audio.py](scripts/patch_audio.py)
to the pinned ESP32-audioI2S 3.0.12 dependency. The patch distinguishes the
terminating zero-size chunk from separators between chunks, drains the buffered
audio, and emits `audio_eof_stream` rather than reconnecting. Chunk extensions
are supported. Ordinary live streams retain reconnect behavior until a valid
terminating chunk is received.

The patch also fixes ring-buffer accounting when the final bytes are consumed.
An empty buffer must not appear full and replay stale audio from the previous
station. The decoder task rechecks playback state after acquiring its mutex.

There is no full-file RAM cache, additional file-size limit, SD requirement or
flash write. The patch is reapplied after dependency installation and is
idempotent; unexpected source layouts fail the build explicitly.

HA `/api/tts_proxy/` URLs received via `/api/play` or MQTT station commands are
handled as announcements, restoring previous playback and radio volume.
HTTP play/announcement requests report connection-start failures with status 502.
Stopping playback cancels an announcement instead of resuming the interrupted
station later.

Patch-generation regression checks can be run after installing the German
environment's dependencies with
`python -B -m unittest discover -s test -p test_patch_audio.py`.
These host checks and a firmware build do not replace testing playback on the radio.
With a native C++ compiler available (`cl` in a Visual Studio developer shell,
`g++`, or `clang++`), the same command also compiles and runs the patched library's
buffer, chunk-parser and EOF handling with deterministic test doubles.

### WAV announcements

The same build-time patch corrects the WAV `data` chunk length (it already
excludes the header), allows short HTTP WAV files to finish header parsing,
and drains the final PCM bytes before reporting EOF. The shared WAV header
parser and final PCM draining are also corrected for SD playback.

For ordinary PCM WAV files served with `Content-Length` (for example by Python's
`SimpleHTTPRequestHandler`), the external Piper script no longer needs to call
`mark_wav_streamable(OUTPUT_FILE)`. Generate and serve the original WAV unchanged.
Existing files using `0xFFFFFFFF` size placeholders remain supported when the
HTTP/file length is known. Supported PCM formats remain 8/16-bit, mono/stereo.
The WAV fix is separate from the chunked-MP3 fix; on-device WAV playback still
needs to be checked after flashing.

Das Topic-Praefix wird beim Einrichten gespeichert. Ohne eigenes Praefix wird standardmaessig ein Geraetename mit Chip-ID verwendet, damit mehrere Radios im selben MQTT-Netz getrennt bleiben.

Beispiele fuer Befehle:

| Topic | Payload |
| --- | --- |
| `<base>/set/power` | `ON` oder `OFF` |
| `<base>/set/volume` | `0` bis `21` |
| `<base>/set/station_name` | exakter Sendername |
| `<base>/set/sd_play` | exakter SD-Dateipfad |
| `<base>/set/alarm_enabled` | `ON` oder `OFF` |
| `<base>/set/alarm_time` | `HH:MM` |
| `<base>/set/alarm_volume` | `0` bis `21` |
| `<base>/set/alarm_source` | `radio` oder `sd` |
| `<base>/set/alarm_sd_path` | exakter SD-Dateipfad |
| `<base>/set/m3u` | Inhalt einer M3U-Liste |

Zustaende werden unter `<base>/state/...` publiziert. Die Verfuegbarkeit steht unter `<base>/availability`.

## Senderlisten

Sender werden ueber M3U-Dateien verwaltet. Ein Eintrag sollte beispielsweise so aussehen:

```text
#EXTM3U
#EXTINF:-1,Deutschlandfunk
https://example.invalid/radio.mp3
```

Die importierten Sender werden dauerhaft im ESP32 gespeichert.

## SD-Dateien

Beim Booten wird die SD-Karte rekursiv durchsucht. Unterstuetzt werden:

- `.mp3`
- `.wav`
- `.aac`
- `.flac`

Wird die SD-Karte im laufenden Betrieb ausgetauscht, muss das Geraet neu gestartet werden, damit die Dateiliste neu erstellt und per MQTT Discovery veroeffentlicht wird.

## Projektstruktur

```text
include/    Konfiguration und Manager fuer WLAN, MQTT, Wecker, Sender und SD
src/        Hauptprogramm
platformio.ini  PlatformIO-Konfiguration
```

## Lizenz

Noch nicht festgelegt.
