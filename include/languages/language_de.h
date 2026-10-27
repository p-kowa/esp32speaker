#pragma once

constexpr const char* SD_NOT_AVAILABLE = "SD-Karte nicht verfuegbar";
constexpr const char* SD_FILE_NOT_FOUND = "SD-Datei nicht gefunden";
constexpr const char* SD_PLAYBACK_FAILED = "Wiedergabe fehlgeschlagen";
constexpr const char* STATION_LIST_WRITE_FAILED = "Senderliste konnte nicht geschrieben werden";
constexpr const char* SD_FILE_ALREADY_EXISTS = "Datei existiert bereits; bitte anderen Namen verwenden";
constexpr const char* SD_FILE_INVALID = "Ungueltiger Dateiname (MP3, WAV, AAC oder FLAC erforderlich)";
constexpr const char* SD_FILE_TOO_LARGE = "Datei zu gross (max. 20 MB)";
constexpr const char* SD_FILE_WRITE_FAILED = "Schreiben fehlgeschlagen (SD voll?)";
constexpr const char* SD_FILE_UPLOAD_ABORTED = "Upload abgebrochen";
constexpr const char* SD_FILE_UPLOAD_INCOMPLETE = "Upload unvollstaendig oder leer";
constexpr const char* SD_FILE_UPLOAD_FAILED = "Upload fehlgeschlagen";
constexpr const char* INVALID_STATION_ID = "Ungueltige Sender-ID";
constexpr const char* INVALID_PARAMETERS = "Ungueltige Parameter";
constexpr const char* NO_M3U_STATIONS_SAVED = "Keine M3U-Sender gespeichert";
constexpr const char* NO_LAST_SELECTED_STATION = "Kein Sender ausgewaehlt";
constexpr const char* ALARM_SD_FILE_NOT_AVAILABLE = "Ausgewaehlte SD-Datei nicht verfuegbar";
constexpr const char* STATION_LIST_CLEAR_FAILED = "Senderliste konnte nicht geloescht werden";
constexpr const char* FAVORITES_LIMIT_PREFIX = "Maximal ";
constexpr const char* FAVORITES_LIMIT_SUFFIX = " Favoriten";
constexpr const char* INVALID_FAVORITE = "Ungueltiger Favorit";
constexpr const char* INVALID_AUDIO_PATH_OR_NO_SD = "Ungueltiger Audiopfad oder keine SD-Karte";
constexpr const char* FILE_NOT_FOUND = "Datei nicht gefunden";
constexpr const char* SD_FILE_IN_USE = "Datei wird abgespielt oder ist als Weckton ausgewaehlt";
constexpr const char* SD_FILE_DELETE_FAILED = "Loeschen fehlgeschlagen";
constexpr const char* SD_UPLOAD_ONE_FILE_ONLY = "Nur eine Datei pro Upload erlaubt";
constexpr const char* NO_SD_CARD = "Keine SD-Karte";
constexpr const char* SD_ALARM_DIR_CREATE_FAILED = "Ordner /alarm konnte nicht angelegt werden";
constexpr const char* SD_UPLOAD_TEMP_CREATE_FAILED = "Upload-Datei konnte nicht erstellt werden";
constexpr const char* SD_FILE_SAVE_FAILED = "Datei konnte nicht gespeichert werden";
constexpr const char* SD_UPLOAD_NO_COMPLETE_FILE = "Keine vollstaendige Datei empfangen";
constexpr const char* SD_FILE_SAVED_PREFIX = "Gespeichert: ";
constexpr const char* NO_PLAYABLE_SD_FILES = "Keine abspielbaren SD-Dateien";
constexpr const char* M3U_BODY_MISSING = "M3U-Inhalt fehlt";
constexpr const char* MQTT_HOST_MISSING = "MQTT-Host fehlt";
constexpr const char* NO_URL_GIVEN = "Keine URL angegeben";
constexpr const char* NO_ANNOUNCE_URL_GIVEN = "Keine Durchsage-URL angegeben";
constexpr const char* INVALID_VALUE = "Ungueltiger Wert";
constexpr const char* NO_STATION_TO_RESUME = "Kein Sender zum Fortsetzen gespeichert";
constexpr const char* MISSING_PARAMETERS = "Fehlende Parameter";
constexpr const char* NOT_FOUND = "Nicht gefunden";
constexpr const char* STATUS_READY_NO_STATION = "Bereit (Kein Sender gewählt)";
constexpr const char* STATUS_NO_TITLE = "Kein Titel";
constexpr const char* STATUS_LOCAL_FILE = "Lokale Datei";
constexpr const char* STATUS_CONNECTING = "Verbinde...";
constexpr const char* STATUS_ANNOUNCEMENT = "📢 DURCHSAGE";
constexpr const char* STATUS_SPEAKING = "Sprachausgabe...";
constexpr const char* STATUS_ALARM_TRIGGERED = "⏰ WECKER AUSGELÖST";
constexpr const char* STATUS_CUSTOM_STREAM = "Eigener Stream";
constexpr const char* STATUS_RADIO = "Radio";
constexpr const char* STATUS_SPEECH_CAPTURE = "Sprachaufnahme";
constexpr const char* STATUS_RECORDING_SPEECH = "Sprache wird aufgenommen...";
constexpr const char* STATUS_SPEECH_COMPLETE = "Sprachaufnahme abgeschlossen";
constexpr const char* STATUS_NO_SPEECH = "Keine Sprache erkannt";
constexpr const char* WAKE_ERROR_MICROPHONE = "Mikrofon: ";
constexpr const char* WAKE_ERROR_NO_SELECTION = "Kein Wake Word gewählt";
constexpr const char* WAKE_ERROR_NO_SD = "Keine SD-Karte";
constexpr const char* WAKE_ERROR_MODEL_MISSING = "Modelldatei fehlt";
constexpr const char* WAKE_ERROR_MODEL_SIZE = "Modelldatei hat ungültige Größe";
constexpr const char* WAKE_ERROR_MODEL_MEMORY = "Kein Speicher für Modell";
constexpr const char* WAKE_ERROR_MODEL_READ = "Modelldatei konnte nicht gelesen werden";
constexpr const char* WAKE_ERROR_INVALID_SETTINGS = "Ungültige Einstellungen";
constexpr const char* WAKE_ERROR_MANIFEST_INVALID = "Modell-JSON fehlt oder ist ungültig";
constexpr const char* WAKE_ERROR_SETTINGS_MISSING = "Einstellungen fehlen in der Modell-JSON";
constexpr const char* WAKE_ERROR_TEMPORARY_CREATE = "Temporäre JSON konnte nicht erstellt werden";
constexpr const char* WAKE_ERROR_JSON_WRITE = "JSON konnte nicht vollständig geschrieben werden";
constexpr const char* WAKE_ERROR_JSON_BACKUP = "Original-JSON konnte nicht gesichert werden";
constexpr const char* WAKE_ERROR_JSON_ACTIVATE = "Neue JSON konnte nicht aktiviert werden";
constexpr const char* WAKE_ERROR_SILENCE_SAVE = "Stillezeit konnte nicht gespeichert werden";
constexpr const char* WAKE_ERROR_TASK_START = "Task konnte nicht gestartet werden";
constexpr const char* WAKE_ERROR_MEMORY = "Kein Speicher";
constexpr const char* WAKE_ERROR_FRONTEND = "Audio-Frontend konnte nicht initialisiert werden";
constexpr const char* WAKE_ERROR_OPERATIONS = "TFLM-Operationen konnten nicht registriert werden";
constexpr const char* WAKE_ERROR_SCHEMA = "Modell-Schema wird nicht unterstützt";
constexpr const char* WAKE_ERROR_ARENA_MEMORY = "Kein Speicher für Tensor-Arena";
constexpr const char* WAKE_ERROR_RESOURCES = "Ressourcen-Variablen konnten nicht angelegt werden";
constexpr const char* WAKE_ERROR_INTERPRETER_MEMORY = "Kein Speicher für Interpreter";
constexpr const char* WAKE_ERROR_TENSORS = "Tensoren konnten nicht angelegt werden (tensor_arena_size zu klein?)";
constexpr const char* WAKE_ERROR_INPUT = "Modell-Eingang passt nicht (erwartet int8 [1, n, 40])";
constexpr const char* WAKE_ERROR_OUTPUT = "Modell-Ausgang passt nicht (erwartet uint8 [1, 1])";
constexpr const char* WAKE_ERROR_EXECUTION = "Modell-Ausführung fehlgeschlagen";
constexpr const char* WAKE_ERROR_UNKNOWN = "Unbekannter Wake-Word-Fehler";
constexpr const char* HA_RADIO_POWER = "Radio Power";
constexpr const char* HA_RADIO_VOLUME = "Radio Lautstärke";
constexpr const char* HA_RADIO_STATION = "Radio Sender";
constexpr const char* HA_RADIO_TITLE = "Radio Titel";
constexpr const char* HA_STATION_SELECT = "Radio Sender Auswahl";
constexpr const char* HA_SD_SELECT = "SD Datei Auswahl";
constexpr const char* HA_ALARM_ENABLED = "Wecker Aktiv";
constexpr const char* HA_ALARM_TIME = "Weckzeit";
constexpr const char* HA_ALARM_VOLUME = "Wecker Lautstärke";
constexpr const char* HA_ALARM_SOURCE = "Wecker Quelle";
constexpr const char* HA_ALARM_SD_FILE = "Wecker SD-Datei";
constexpr const char* HA_ANNOUNCE_VOLUME = "Durchsage Lautstärke";
constexpr const char* HA_ANNOUNCE_STATUS = "Durchsage Status";
#define T_ALARM_LABEL_SD_FILE_PREFIX "SD-Datei: "
constexpr const char* ALARM_LABEL_SD_FILE_PREFIX = T_ALARM_LABEL_SD_FILE_PREFIX;
constexpr const char* ALARM_LABEL_RADIO_PREFIX = "Radio: ";
constexpr const char* ALARM_LABEL_RADIO_NOT_SET = "Radio: nicht gesetzt";

// Web interface texts (string macros, concatenated at compile time in web_pages.h).
// Must not contain quotes, backslashes or line breaks (some are used inside JavaScript strings).
#define T_HTML_LANG "de"
#define T_LOADING "Lade..."
#define T_ALARM_PAGE_TITLE "Wecker - ESP32 Radio"
#define T_ALARM_HEADING "⏰ Radio-Wecker"
#define T_ALARM_SUBTITLE "Echtzeit-Uhrzeit & Weckeinstellungen"
#define T_ALARM_CLOCK_LABEL "ESP32 Uhrzeit (NTP)"
#define T_ALARM_CLOCK_SUFFIX " Uhr"
#define T_ALARM_CURRENT_SOUND "Aktueller Weckton"
#define T_ALARM_TIME_LABEL "Weckzeit"
#define T_ALARM_ENABLE "Wecker aktivieren"
#define T_ALARM_VOLUME_LABEL "Weck-Lautstärke"
#define T_ALARM_SAVE_BUTTON "💾 Wecker speichern"
#define T_ALARM_NEED_STATIONS_MAIN "Bitte zuerst eine M3U-Senderliste auf der Hauptseite hochladen."
#define T_ALARM_TEST_HEADING "Testen"
#define T_ALARM_TEST_BUTTON "🔔 Wecker jetzt sofort testen"
#define T_ALARM_SAVED_MSG "✅ Wecker-Einstellungen gespeichert!"
#define T_ALARM_TRIGGERED_MSG "🔔 Weck-Signal ausgelöst!"
#define T_ALARM_STOP_TEST_BUTTON "⏹ Weckertest beenden"
#define T_ALARM_STOPPED_MSG "⏹ Weckertest beendet."
#define T_ALARM_NEED_STATIONS "Bitte zuerst eine M3U-Senderliste hochladen."

#define T_SD_PAGE_TITLE "MicroSD - ESP32 Radio"
#define T_SD_HEADING "MicroSD-Dateien"
#define T_SD_INTRO "Lokale Audiodateien abspielen oder als Weckerquelle speichern."
#define T_SD_PLAY_ALL "▶ Alle abspielen"
#define T_SD_STOP_ALL "■ Alle stoppen"
#define T_SD_REFRESH "↻ Aktualisieren"
#define T_SD_UPLOAD_HEADING "Weckton hochladen"
#define T_SD_FILE_ARIA "Weckton auswählen"
#define T_SD_UPLOAD_BUTTON "Weckton hochladen"
#define T_SD_FILES_HEADING "Audiodateien"
#define T_SD_LOADING_FILES "Dateien werden geladen ..."
#define T_BACK_TO_RADIO "Zurück zum Radio"
#define T_SD_ACTION_FAILED "Aktion fehlgeschlagen"
#define T_SD_PLAY_TITLE "Auf dem Gerät abspielen"
#define T_SD_PLAY_STARTED "Wiedergabe gestartet."
#define T_SD_SET_ALARM_TITLE "Als Weckton speichern"
#define T_SD_ALARM_SAVED "Datei als Weckerquelle gespeichert."
#define T_SD_DOWNLOAD_TITLE "Herunterladen"
#define T_SD_DELETE_TITLE "Löschen"
#define T_SD_DELETE_CONFIRM_PREFIX "Datei "
#define T_SD_DELETE_CONFIRM_SUFFIX " löschen?"
#define T_SD_DELETED "Datei gelöscht."
#define T_SD_LIST_UNREACHABLE "SD-Liste nicht erreichbar"
#define T_SD_NO_FILES "Keine abspielbaren Dateien gefunden oder keine SD-Karte eingelegt."
#define T_SD_FILES_FOUND_PREFIX ""
#define T_SD_FILES_FOUND_SUFFIX " Datei(en) gefunden."
#define T_SD_LOAD_FAILED "SD-Dateien konnten nicht geladen werden."
#define T_SD_UPLOAD_SIZE_HINT "Bitte eine Datei mit 1 Byte bis 20 MB auswählen."
#define T_SD_UPLOAD_RUNNING "Upload läuft..."
#define T_SD_PLAY_ALL_MSG "Alle Dateien werden nacheinander abgespielt."
#define T_SD_STOPPED "Wiedergabe gestoppt."

#define T_SETUP_PAGE_TITLE "WLAN Konfiguration - ESP32 Radio"
#define T_SETUP_HEADING "⚙️ WLAN-Einstellungen"
#define T_SETUP_SUBTITLE "Verbinde dein ESP32 Radio mit dem Heimnetz"
#define T_SETUP_CONNECTED_PREFIX "Verbunden mit "
#define T_SETUP_CONNECTED_SUFFIX ""
#define T_SETUP_NETWORKS_LABEL "Verfügbare Netzwerke (2.4 GHz)"
#define T_SETUP_SCAN_BUTTON "🔄 Scan"
#define T_SETUP_SCAN_HINT "Zum Suchen auf Scan klicken"
#define T_SETUP_MANUAL_SSID_LABEL "Oder SSID manuell eingeben"
#define T_SETUP_SSID_PLACEHOLDER "WLAN-Name (SSID)"
#define T_SETUP_WIFI_PASS_LABEL "WLAN Passwort"
#define T_SETUP_WIFI_PASS_PLACEHOLDER "Passwort eingeben"
#define T_SETUP_SAVE_CONNECT "💾 Speichern & Verbinden"
#define T_SETUP_OPTIONS_HEADING "Optionen"
#define T_SETUP_RESET_BUTTON "🗑️ Gekoppelte WLAN-Daten löschen"
#define T_SETUP_MQTT_HOST_LABEL "Broker-Adresse"
#define T_SETUP_MQTT_USER_LABEL "Benutzername (optional)"
#define T_SETUP_MQTT_LEAVE_EMPTY "leer lassen möglich"
#define T_SETUP_MQTT_PASS_LABEL "Passwort (optional)"
#define T_SETUP_MQTT_BASE_LABEL "MQTT Topic-Basis"
#define T_SETUP_MQTT_SAVE "💾 Einstellungen speichern"
#define T_SETUP_MQTT_STATUS_LOADING "MQTT-Status wird geladen..."
#define T_SETUP_SEARCHING_NETWORKS "Suche Netzwerke..."
#define T_SETUP_SCANNING "Scanne WLANs..."
#define T_SETUP_SELECT_NETWORK "-- Netzwerk auswählen --"
#define T_SETUP_NO_NETWORKS "Keine Netzwerke gefunden"
#define T_SETUP_NETWORKS_FOUND_PREFIX ""
#define T_SETUP_NETWORKS_FOUND_SUFFIX " Netzwerke gefunden."
#define T_SETUP_SCAN_ERROR "Fehler beim Scannen der Netzwerke."
#define T_SETUP_SSID_REQUIRED "Bitte gib einen WLAN-Namen (SSID) an."
#define T_SETUP_SAVING_PREFIX "Speichere und verbinde mit "
#define T_SETUP_SAVING_SUFFIX "..."
#define T_SETUP_SAVED "✅ Gespeichert! Der ESP32 verbindet sich jetzt neu..."
#define T_SETUP_SAVED_HOTSPOT_HINT "Falls du im Hotspot-Modus warst, verbinde dein Handy jetzt wieder mit deinem normalen WLAN."
#define T_SETUP_SAVE_ERROR "Fehler beim Speichern der Daten."
#define T_SETUP_RESET_CONFIRM "Möchtest du die gespeicherten WLAN-Zugangsdaten wirklich löschen?"
#define T_SETUP_RESET_DONE "WLAN-Daten gelöscht. Startet neu..."
#define T_SETUP_MQTT_DISABLED "MQTT deaktiviert"
#define T_SETUP_MQTT_CONNECTED_PREFIX "MQTT zu "
#define T_SETUP_MQTT_CONNECTED_SUFFIX " verbunden"
#define T_SETUP_MQTT_DISCONNECTED_PREFIX "MQTT zu "
#define T_SETUP_MQTT_DISCONNECTED_SUFFIX " getrennt"
#define T_SETUP_MQTT_UNKNOWN "MQTT-Status unbekannt"
#define T_SETUP_MQTT_UNREACHABLE "MQTT-Status nicht erreichbar"
#define T_SETUP_MQTT_SAVED "MQTT gespeichert. Verbindung wird aufgebaut."
#define T_SETUP_WAV_TRANSPORT_LABEL "Übertragungsmethode für WAV"
#define T_SETUP_WAV_MQTT_TOPIC_LABEL "MQTT Topic für WAV-Nachrichten"
#define T_SETUP_WAV_HTTP_URL_LABEL "HTTP-URL für WAV-Nachrichten"
#define T_SETUP_WAV_HTTP_HINT "Nur http:// wird unterstützt. Verwende die IP-Adresse des Whisper-Servers, nicht localhost."
#define T_SETUP_HTTP_URL_REQUIRED "Bitte eine HTTP-URL beginnend mit http:// eingeben."
#define T_SETUP_WYOMING_HOST_REQUIRED "Bitte einen Wyoming-Server eingeben."
#define T_SETUP_SAVE_FAILED_PREFIX "Speichern fehlgeschlagen: "

#define T_IDX_OPTIONS "Optionen"
#define T_IDX_NAV_ALARM "⏰ Wecker"
#define T_IDX_NAV_WIFI "⚙️ WLAN"
#define T_IDX_STOPPED "Gestoppt"
#define T_IDX_BITRATE_LABEL "Bitrate: "
#define T_IDX_VOLUME "Lautstärke"
#define T_IDX_STOP_BUTTON "⏹ Stop"
#define T_IDX_PLAY_BUTTON "▶ Play"
#define T_IDX_FAVORITES_HEADING "Favoriten"
#define T_IDX_SHOW_ALL "Alle Sender anzeigen"
#define T_IDX_SAVE_AS_ALARM "⏰ Als Weckton speichern"
#define T_IDX_STATION_LIST_HEADING "Senderliste"
#define T_IDX_SEARCH_PLACEHOLDER "Sender suchen..."
#define T_IDX_CUSTOM_URL_HEADING "Eigene Stream-URL"
#define T_IDX_CUSTOM_PLAY "Play"
#define T_IDX_ANNOUNCE_VOLUME "📢 Durchsage-Lautstärke (TTS)"
#define T_IDX_M3U_HEADING "M3U-Senderliste"
#define T_IDX_M3U_APPEND "An bestehende Liste anhängen"
#define T_IDX_M3U_REPLACE "Bestehende Liste ersetzen"
#define T_IDX_M3U_UPLOAD "📂 M3U hochladen"
#define T_IDX_CLEAR_ALL "🗑️ Alle Sender löschen"
#define T_IDX_NO_FAVORITES "Noch keine Favoriten – in der Senderliste mit ☆ markieren."
#define T_IDX_LOAD_ERROR "Fehler beim Laden"
#define T_IDX_NO_MATCHES "Keine Treffer."
#define T_IDX_NO_STATIONS_PREFIX "Keine Sender gespeichert – M3U-Liste unter "
#define T_IDX_NO_STATIONS_SUFFIX " hochladen."
#define T_IDX_PLAY_TITLE "Abspielen"
#define T_IDX_FAV_REMOVE "Aus Favoriten entfernen"
#define T_IDX_FAV_ADD "Zu Favoriten hinzufügen"
#define T_IDX_DELETE_TITLE "Löschen"
#define T_IDX_PAGE_OF " von "
#define T_IDX_NO_SD_LIMIT_SHORT " (ohne SD-Karte max. 24)"
#define T_IDX_FAV_CHANGE_FAILED "Favorit konnte nicht geändert werden: "
#define T_IDX_DELETE_CONFIRM_PREFIX "Sender "
#define T_IDX_DELETE_CONFIRM_SUFFIX " löschen?"
#define T_IDX_STATION_DELETED "Sender gelöscht."
#define T_IDX_DELETE_FAILED "Löschen fehlgeschlagen: "
#define T_IDX_CLEAR_CONFIRM "Wirklich ALLE Sender löschen? Favoriten bleiben erhalten."
#define T_IDX_CLEARED "Alle Sender gelöscht."
#define T_IDX_M3U_SELECT_FILE "Bitte zuerst eine M3U-Datei auswählen."
#define T_IDX_M3U_UPLOADING "M3U wird hochgeladen..."
#define T_IDX_IMPORTED_PREFIX ""
#define T_IDX_IMPORTED_MID " Sender importiert, insgesamt "
#define T_IDX_IMPORTED_SUFFIX "."
#define T_IDX_NVS_LIMIT_NOTE " Ohne SD-Karte werden max. 24 Sender gespeichert."
#define T_IDX_UPLOAD_FAILED "Upload fehlgeschlagen: "
#define T_IDX_ALARM_STATION_SAVED "Aktueller Sender ist jetzt der Weckton."
#define T_IDX_ALARM_STATION_FAILED "Weckton konnte nicht gespeichert werden: "
#define T_IDX_UNKNOWN "Unbekannt"
#define T_IDX_BADGE_ANNOUNCING "📢 Durchsage"
#define T_IDX_BADGE_PLAYING "Wiedergabe"

#define T_IDX_SPEECH_RECORDING "Sprachaufnahme läuft"
#define T_IDX_WAKE_WORD_SETTINGS_MISSING "Wake-Word-Einstellungen fehlen"
#define T_IDX_WAKE_WORD_SETTINGS_INVALID "Ungueltige Wake-Word-Einstellungen"
#define T_IDX_WAKE_WORD_MODEL_NOT_FOUND "Modell nicht gefunden"
#define T_IDX_I2S_MIC_START_FAILED "I2S-Mikrofon konnte nicht gestartet werden: "

#define T_IDX_SUBTITLE "Audio-Streaming"
#define T_SETUP_PORT_LABEL "Port"
#define T_ALARM_STATION_LIST_UNAVAILABLE "Senderliste nicht verfügbar"

#define T_WW_NAME "Wake Word"
#define T_WW_MODELS_INTRO "microWakeWord-Modelle"
#define T_WW_FOLDER_INTRO "im Ordner"
#define T_WW_SD_INTRO "der MicroSD-Karte."
#define T_WW_ACTIVE "Aktiv"
#define T_WW_DETECTION_LABEL "Erkennung: "
#define T_WW_PROBABILITY_LABEL "Wahrscheinlichkeit: "
#define T_WW_DISABLE "Wake Word deaktivieren"
#define T_WW_LOADING_MODELS "Modelle werden geladen ..."
#define T_WW_DETECTION_HEADING "Wake-Word-Erkennung"
#define T_WW_THRESHOLD "Schwellwert"
#define T_WW_THRESHOLD_HINT "Je höher, desto sicherer muss das Wake Word erkannt werden."
#define T_WW_WINDOW_SIZE "Fenstergröße"
#define T_WW_WINDOW_HINT "Anzahl der Auswertungen für die Mittelung."
#define T_WW_SILENCE_LABEL "Stille bis Satzende"
#define T_WW_SILENCE_HINT "Stillezeit nach dem Sprechen, bevor die Aufnahme endet."
#define T_WW_SAVE_SETTINGS "Wake-Word-Einstellungen speichern"
#define T_WW_MISSING_FILE_PREFIX ""
#define T_WW_MISSING_FILE_SUFFIX " fehlt"
#define T_WW_USE "Verwenden"
#define T_WW_NONE "keins"
#define T_WW_NO_SD "Keine MicroSD-Karte eingelegt."
#define T_WW_SD_UNAVAILABLE "MicroSD-Karte nicht verfügbar."
#define T_WW_FOLDER_FAILED_PREFIX "Ordner "
#define T_WW_FOLDER_FAILED_SUFFIX " konnte nicht angelegt werden."
#define T_WW_FOLDER_ERROR "Ordnerfehler auf der MicroSD-Karte."
#define T_WW_NO_MODELS_PREFIX "Keine Modelle gefunden. Kopiere name.tflite und name.json nach "
#define T_WW_LOAD_FAILED "Modelle konnten nicht geladen werden."
#define T_WW_SELECT_FAILED "Auswahl fehlgeschlagen"
#define T_WW_SELECTED_PREFIX "Wake Word "
#define T_WW_SELECTED_SUFFIX " gespeichert."
#define T_WW_DISABLED "Wake Word deaktiviert."
#define T_WW_RUNNING "läuft"
#define T_WW_OFF "aus"
#define T_WW_DETECTED_LABEL "erkannt: "
#define T_WW_DETECTED "Wake Word erkannt!"
#define T_WW_STATUS_UNREACHABLE "Erkennung: Status nicht erreichbar"
#define T_WW_SETTINGS_SAVED "Einstellungen gespeichert und Erkennung neu gestartet."
#define T_WW_SERVER_UNREACHABLE "Server nicht erreichbar"

#define T_SETUP_WAV_WYOMING_LABEL "Wyoming Einstellungen"