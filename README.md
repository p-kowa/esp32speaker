# ESP32 Speaker

ESP32-S3 internet radio, MicroSD audio player and wake word receiver built with
PlatformIO, Arduino and ESP-IDF. Control playback through the web interface,
MQTT or Home Assistant. Play URL-based announcements and send speech recorded
after wake word detection to an external service.

## Features

- Internet radio with M3U import, station search and favorites.
- MAX98357A I2S speaker output.
- MicroSD playback of MP3, WAV, AAC and FLAC files.
- Web-based audio upload, download, deletion and sequential playback.
- Alarm with a radio or SD source, separate volume and start/stop test.
- URL-based announcements with separate volume and restoration of previous
  playback.
- MQTT control, availability reporting and Home Assistant discovery.
- Local microWakeWord inference using an ICS43434 I2S microphone.
- Post-detection speech capture with WAV upload over MQTT or HTTP POST, or
  transcription through a Wyoming server.
- German, English and Polish firmware variants.
- Persistent Wi-Fi, MQTT, alarm and wake word settings.

## Hardware

The current configuration targets an ESP32-S3 N16R8 board: **16 MB flash and
8 MB octal PSRAM**, using the ESP32-S3 DevKitC-1 board definition.

| Connection | ESP32-S3 GPIO |
| --- | ---: |
| MAX98357A BCLK | 9 |
| MAX98357A LRC / WS | 7 |
| MAX98357A DIN (ESP32 DOUT) | 8 |
| MicroSD SCK | 12 |
| MicroSD MOSI | 11 |
| MicroSD MISO | 13 |
| MicroSD CS | 10 |
| ICS43434 BCLK | 14 |
| ICS43434 LRC / WS | 16 |
| ICS43434 DOUT (ESP32 input) | 21 |
| Status LED | 15 |

Connect the ICS43434 L/R pin to GND for the left audio slot. Pin assignments
and default audio volumes are defined in [include/config.h](include/config.h).

Required for radio playback: the ESP32-S3, MAX98357A amplifier, speaker and Wi-Fi.
A SPI MicroSD reader and card provide local audio and model storage. Wake word
operation additionally requires the microphone and a compatible model.
MQTT and Home Assistant are optional.

## Build and upload

Install PlatformIO, open this project and select the desired environment under
**PlatformIO > Project Tasks**.

| Language | Environment |
| --- | --- |
| German (default) | `esp32-s3-n16r8_DE` |
| English | `esp32-s3-n16r8_EN` |
| Polish | `esp32-s3-n16r8_PL` |

For example, build and upload **only the German version**:

```powershell
pio run -e esp32-s3-n16r8_DE -j 2
pio run -e esp32-s3-n16r8_DE -t upload -j 2
pio device monitor -b 115200
```

Replace the environment name with `_EN` or `_PL` for another language.
Language is selected at compile time, not by a runtime language menu.
Plain `pio run` uses the German environment configured in `default_envs`.

The first build compiles ESP-IDF, Arduino, TFLite Micro and ESP-NN and can take
a considerable amount of time. `-j 2` limits build concurrency; use `-j 1` only
if lower resource usage is necessary, at the cost of a longer build.
Avoid simultaneous builds and do not clean routinely: incremental builds reuse
existing results. A clean forces a full rebuild of the selected environment.

See [platformio.ini](platformio.ini) for dependencies and board settings.
The current audio library is ESP32-audioI2S 3.0.12. Keep `TF_LITE_STATIC_MEMORY`
consistent with the TFLite Micro component build because it affects tensor layout.

## Initial setup

1. Flash the firmware.
2. If Wi-Fi is not configured or a connection cannot be established, connect to
   the setup access point:
   - SSID: `ESP32-Radio-Setup`
   - Password: `radio-setup`
3. Open the access-point IP shown in the serial log and configure Wi-Fi.
4. Open the radio's IP address on your LAN.
5. Import an M3U list on the main page.
6. Optionally configure MQTT and speech-upload transport on `/setup`.
7. For local playback and wake word models, insert a prepared MicroSD card before
   booting. Restart after replacing the card.

### Web pages

| Path | Purpose |
| --- | --- |
| `/` | Radio, favorites, station import, stream URL and volume controls |
| `/setup` | Wi-Fi, MQTT and outgoing speech-upload settings |
| `/alarm` | Alarm time, enable switch, volume and playback test |
| `/sdcard` | Audio file management and playback |
| `/wakeword` | Model selection, detection status and model settings; shows the latest transcript when Wyoming is active |

UI and API messages follow the firmware language. Diagnostic log messages are
English. Native file-picker labels and the system file dialog follow the browser
or operating-system language.

## Stations and MicroSD audio

Import an M3U list through the main page, replacing or appending to the current
list. For example:

```text
#EXTM3U
#EXTINF:-1,Example Station
http://example.invalid/radio.mp3
```

Station storage uses the SD card when available, with a fallback of up to
24 stations in NVS without a card. Up to 50 favorites are supported.

The SD page supports `.mp3`, `.wav`, `.aac` and `.flac`, with uploads of up to
20 MiB per file. You can play a file, play all files, download or delete audio,
and select an alarm sound. The `/wakeword` directory is excluded from normal
audio-file management to protect models and related files.

The alarm supports a stored radio station or an SD file and synchronizes time
using NTP with Central European CET/CEST rules. Its test button toggles playback;
volume changes can be previewed during an alarm, while saving persists settings.

## Announcements and external TTS

The firmware plays an **audio URL**, not raw text. An external TTS service must
generate and serve the audio. Piper can run on a separate Linux host or LXC;
it does not have to run on the Home Assistant machine.

For a Linux HTTP test:

```bash
curl --get "http://RADIO_IP/api/announce" \
  --data-urlencode "url=http://AUDIO_SERVER_IP:8000/message.mp3" \
  --data-urlencode "volume=15"
```

`/api/announce` accepts GET or POST form parameters. `url` is required;
`volume` (or `vol`) is optional and ranges from 0 to 21. Without an override,
the saved announcement volume is used.

The radio downloads the audio directly, interrupts current playback, and restores
the saved playback state and volume afterward. A new announcement during an
existing announcement replaces it without taking another playback snapshot.
The firmware includes a 10-second inactivity timeout and a five-minute maximum
announcement duration.

Use a server address reachable **from the radio**, not `localhost`.

### Audio library fixes and Home Assistant TTS

The build automatically applies [scripts/patch_audio.py](scripts/patch_audio.py)
to the pinned ESP32-audioI2S 3.0.12 dependency. The patch is idempotent and is
reapplied after dependency installation; unexpected source layouts fail the
build explicitly.

Home Assistant may serve `/api/tts_proxy/*.mp3` using HTTP chunked transfer
without a `Content-Length` header. The patch distinguishes the terminating
zero-size chunk from separators between chunks, drains the buffered audio and
emits `audio_eof_stream` instead of repeatedly reconnecting to the same TTS URL.
Chunk extensions are supported. Ordinary live streams retain reconnect behavior
until a valid terminating chunk is received.

The patch also fixes ring-buffer accounting so a fully consumed buffer is
recognized as empty rather than replaying stale audio. The decoder task rechecks
playback state after acquiring its mutex.

Home Assistant `/api/tts_proxy/` URLs received through `/api/play` or MQTT station
URL commands are handled as announcements, restoring previous playback and radio
volume afterward. HTTP announcement requests, including these TTS requests,
report connection-start failures with status 502. Stopping playback cancels an
announcement instead of resuming the interrupted station later.

There is no full-file RAM cache, additional file-size limit, SD requirement or
flash write for URL-based announcements.

### WAV announcements

The same library patch corrects the WAV `data` chunk length, which already
excludes the header. It allows short HTTP WAV files to finish header parsing and
drains the final PCM bytes before reporting EOF. The shared WAV header parser
and final PCM draining are also corrected for SD playback.

For ordinary PCM WAV files served with `Content-Length`, for example by Python's
`SimpleHTTPRequestHandler`, generate and serve the original WAV unchanged.
The external Piper script no longer needs to call
`mark_wav_streamable(OUTPUT_FILE)` or replace the RIFF and `data` chunk sizes.
Existing files using `0xFFFFFFFF` size placeholders remain supported when the
HTTP/file length is known. Supported PCM formats remain 8/16-bit, mono/stereo.

After flashing, test an unmodified WAV announcement and confirm that playback
returns to the previous station and volume. Host regression checks do not replace
playback testing on the device.
The external Python service is not included in this repository.

## Wake word detection and speech upload

Copy a compatible microWakeWord model and its matching manifest to the SD card:

```text
/wakeword/name.tflite
/wakeword/name.json
```

Use a model with `int8 [1, n, 40]` input and `uint8 [1, 1]` output. Arbitrary
TFLite speech models are not interchangeable with microWakeWord models.

On `/wakeword`, select a model, monitor detection probability and adjust:

- Probability threshold.
- Sliding-window size for averaging.
- Silence duration after speech before recording ends.

The manifest supplies `probability_cutoff`, `sliding_window_size` and, when
present, `tensor_arena_size` and `feature_step_size`. Saving threshold/window
settings updates the selected model's JSON; the silence setting is stored in NVS.

After detection, the radio pauses playback and records speech at 16 kHz,
16-bit mono. Capture ends according to speech/silence detection or after a maximum
of 10 seconds. Recordings without detected speech are discarded. Speech is sent
as WAV using the transport selected on `/setup`:

| Transport | Configuration | Payload |
| --- | --- | --- |
| MQTT | Broker and outgoing WAV topic | Binary WAV, not JSON or Base64 |
| HTTP POST | Destination URL starting with `http://` | WAV body with `Content-Type: audio/wav` |
| Wyoming | Server host and port (default `10300`) | Wyoming TCP events with normalized 16 kHz mono PCM; the transcript is logged, shown on `/wakeword` and published to `<base>/state/transcript` when MQTT is connected |

The MQTT receiver must accept the full WAV payload; configure MQTT message-size
limits accordingly. The default outgoing topic is `esp32radio/audio/upload`;
configure distinct topics for multiple radios if the receiving service needs to
identify the source. HTTP speech upload currently supports plain HTTP, not HTTPS.
Wyoming mode connects directly to the Wyoming service and requires the server to
be reachable from the radio on the configured TCP port.

An external service must perform speech recognition and any subsequent action.
The firmware does not perform transcription or model training.
The previous training-recording UI and backend have been removed; model training
is performed externally, for example in Google Colab. Normal speech capture after
wake word detection remains available.

## MQTT

Configure the broker and topic base on `/setup`. On first use, the default base
includes a chip-derived device ID. Give each radio a distinct base if configuring
it manually.

| Command topic | Payload |
| --- | --- |
| `<base>/set/power` | `ON` or `OFF` |
| `<base>/set/volume` | `0` to `21` |
| `<base>/set/station_name` | Exact name of a favorite station |
| `<base>/set/station_url` | Stream URL |
| `<base>/set/sd_play` | SD audio path |
| `<base>/set/announce` | Audio URL or `{"url":"http://host/message.mp3","volume":15}` |
| `<base>/set/announce_volume` | `0` to `21` |
| `<base>/set/alarm_enabled` | `ON` or `OFF` |
| `<base>/set/alarm_time` | `HH:MM` |
| `<base>/set/alarm_volume` | `0` to `21` |
| `<base>/set/alarm_source` | `radio` or `sd` |
| `<base>/set/alarm_sd_path` | SD audio path |
| `<base>/set/m3u` | M3U list contents |

Playback and alarm states are published under `<base>/state/...`.
Availability is published under `<base>/availability`.
Do not retain announcement commands, to avoid replaying them after reconnecting.
The outgoing microphone WAV topic is configured separately from this command base.

## Home Assistant

Connect Home Assistant to the same MQTT broker. The radio publishes discovery
configuration under the `homeassistant` prefix and appears as an
`ESP32 Radio (...)` device.

Discovered entities include radio power, volume, station/title sensors,
station and SD playback selectors, alarm controls, announcement volume and
announcement-active status. Names follow the firmware language.
List-based selectors depend on available station/audio entries.
Selecting an SD playback entry starts playback; selecting an alarm SD entry
configures the alarm file instead. Set the alarm source to `sd` to use it.

To announce an existing audio URL from an automation:

```yaml
action: mqtt.publish
data:
  topic: "YOUR_RADIO_BASE/set/announce"
  payload: '{"url":"http://AUDIO_SERVER_IP:8000/message.mp3","volume":15}'
  retain: false
```

The firmware's MQTT discovery does **not** itself provide a Home Assistant
`media_player` entity or an ESPHome voice-assistant satellite implementation.
A compatible external Home Assistant media-player integration can provide the
entity needed for `tts.speak` and forward the resulting audio URL to the radio.
Home Assistant `/api/tts_proxy/` URLs are automatically treated as announcements
when passed through `/api/play` or MQTT station URL commands.

Piper running in an LXC can separately be exposed to HA through a Wyoming
service; that alone does not add media-player support to the radio.

## Network access

Use the device and its audio/upload services on a trusted network. The HTTP
control API has no authentication; do not expose it directly to the internet.
Use broker authentication and appropriate MQTT permissions where available.

## Project structure

| Path | Contents |
| --- | --- |
| [src/](src/) | Main application, microphone, wake word detector and tone generation |
| [include/](include/) | Configuration, web pages and managers |
| [include/languages/](include/languages/) | German, English and Polish texts |
| [components/](components/) | TFLite Micro and ESP-NN components |
| [platformio.ini](platformio.ini) | Build environments and dependencies |
| [n16r8_part.csv](n16r8_part.csv) | Flash partition layout |

## License

A project-wide license has not yet been specified. Bundled third-party components
retain their respective licenses; see their directories.
