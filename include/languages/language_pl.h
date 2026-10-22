#pragma once

constexpr const char* SD_NOT_AVAILABLE = "Karta SD niedostępna";
constexpr const char* SD_FILE_NOT_FOUND = "Nie znaleziono pliku SD";
constexpr const char* SD_PLAYBACK_FAILED = "Odtwarzanie nie powiodło się";
constexpr const char* SD_FILE_ALREADY_EXISTS = "Plik już istnieje; użyj innej nazwy";
constexpr const char* SD_FILE_INVALID = "Nieprawidłowa nazwa pliku (wymagany MP3, WAV, AAC lub FLAC)";
constexpr const char* SD_FILE_TOO_LARGE = "Plik jest za duży (max. 20 MB)";
constexpr const char* SD_FILE_WRITE_FAILED = "Błąd zapisu (karta SD pełna?)";
constexpr const char* SD_FILE_UPLOAD_ABORTED = "Przesyłanie przerwane";
constexpr const char* SD_FILE_UPLOAD_INCOMPLETE = "Przesyłanie niekompletne lub puste";
constexpr const char* SD_FILE_UPLOAD_FAILED = "Przesyłanie nie powiodło się";
constexpr const char* STATION_LIST_WRITE_FAILED = "Nie udało się zapisać listy stacji";
constexpr const char* INVALID_STATION_ID = "Nieprawidłowy identyfikator stacji";
constexpr const char* INVALID_PARAMETERS = "Nieprawidłowe parametry";
constexpr const char* NO_M3U_STATIONS_SAVED = "Brak zapisanych stacji M3U";
constexpr const char* NO_LAST_SELECTED_STATION = "Brak wybranej stacji";
constexpr const char* ALARM_SD_FILE_NOT_AVAILABLE = "Wybrany plik SD jest niedostępny";
constexpr const char* STATION_LIST_CLEAR_FAILED = "Nie udało się wyczyścić listy stacji";
constexpr const char* FAVORITES_LIMIT_PREFIX = "Maksymalnie ";
constexpr const char* FAVORITES_LIMIT_SUFFIX = " ulubionych";
constexpr const char* INVALID_FAVORITE = "Nieprawidłowa ulubiona stacja";
constexpr const char* INVALID_AUDIO_PATH_OR_NO_SD = "Nieprawidłowa ścieżka audio lub brak karty SD";
constexpr const char* FILE_NOT_FOUND = "Nie znaleziono pliku";
constexpr const char* SD_FILE_IN_USE = "Plik jest odtwarzany lub wybrany jako dźwięk budzika";
constexpr const char* SD_FILE_DELETE_FAILED = "Usuwanie nie powiodło się";
constexpr const char* SD_UPLOAD_ONE_FILE_ONLY = "Można przesłać tylko jeden plik naraz";
constexpr const char* NO_SD_CARD = "Brak karty SD";
constexpr const char* SD_ALARM_DIR_CREATE_FAILED = "Nie udało się utworzyć folderu /alarm";
constexpr const char* SD_UPLOAD_TEMP_CREATE_FAILED = "Nie udało się utworzyć pliku tymczasowego";
constexpr const char* SD_FILE_SAVE_FAILED = "Nie udało się zapisać pliku";
constexpr const char* SD_UPLOAD_NO_COMPLETE_FILE = "Nie otrzymano kompletnego pliku";
constexpr const char* SD_FILE_SAVED_PREFIX = "Zapisano: ";
constexpr const char* NO_PLAYABLE_SD_FILES = "Brak plików SD do odtworzenia";
constexpr const char* M3U_BODY_MISSING = "Brak treści M3U";
constexpr const char* MQTT_HOST_MISSING = "Brak hosta MQTT";
constexpr const char* NO_URL_GIVEN = "Nie podano adresu URL";
constexpr const char* NO_ANNOUNCE_URL_GIVEN = "Nie podano adresu URL komunikatu";
constexpr const char* INVALID_VALUE = "Nieprawidłowa wartość";
constexpr const char* NO_STATION_TO_RESUME = "Brak zapisanej stacji do wznowienia";
constexpr const char* MISSING_PARAMETERS = "Brak parametrów";
constexpr const char* NOT_FOUND = "Nie znaleziono";
constexpr const char* STATUS_READY_NO_STATION = "Gotowe (nie wybrano stacji)";
constexpr const char* STATUS_NO_TITLE = "Brak tytułu";
constexpr const char* STATUS_LOCAL_FILE = "Plik lokalny";
constexpr const char* STATUS_CONNECTING = "Łączenie...";
constexpr const char* STATUS_ANNOUNCEMENT = "📢 KOMUNIKAT";
constexpr const char* STATUS_SPEAKING = "Odtwarzanie komunikatu...";
constexpr const char* STATUS_ALARM_TRIGGERED = "⏰ BUDZIK URUCHOMIONY";
constexpr const char* STATUS_CUSTOM_STREAM = "Własny strumień";
constexpr const char* STATUS_RADIO = "Radio";
constexpr const char* HA_RADIO_POWER = "Zasilanie radia";
constexpr const char* HA_RADIO_VOLUME = "Głośność radia";
constexpr const char* HA_RADIO_STATION = "Stacja radiowa";
constexpr const char* HA_RADIO_TITLE = "Tytuł utworu";
constexpr const char* HA_STATION_SELECT = "Wybór stacji radiowej";
constexpr const char* HA_SD_SELECT = "Wybór pliku SD";
constexpr const char* HA_ALARM_ENABLED = "Budzik aktywny";
constexpr const char* HA_ALARM_TIME = "Godzina budzenia";
constexpr const char* HA_ALARM_VOLUME = "Głośność budzika";
constexpr const char* HA_ALARM_SOURCE = "Źródło budzika";
constexpr const char* HA_ALARM_SD_FILE = "Plik SD budzika";
constexpr const char* HA_ANNOUNCE_VOLUME = "Głośność komunikatów";
constexpr const char* HA_ANNOUNCE_STATUS = "Status komunikatu";
#define T_ALARM_LABEL_SD_FILE_PREFIX "Plik SD: "
constexpr const char* ALARM_LABEL_SD_FILE_PREFIX = T_ALARM_LABEL_SD_FILE_PREFIX;
constexpr const char* ALARM_LABEL_RADIO_PREFIX = "Radio: ";
constexpr const char* ALARM_LABEL_RADIO_NOT_SET = "Radio: nie ustawiono";

// Web interface texts (string macros, concatenated at compile time in web_pages.h).
// Must not contain quotes, backslashes or line breaks (some are used inside JavaScript strings).
#define T_HTML_LANG "pl"
#define T_LOADING "Ładowanie..."
#define T_ALARM_PAGE_TITLE "Budzik - ESP32 Radio"
#define T_ALARM_HEADING "⏰ Budzik radiowy"
#define T_ALARM_SUBTITLE "Czas i ustawienia budzika"
#define T_ALARM_CLOCK_LABEL "Czas ESP32 (NTP)"
#define T_ALARM_CLOCK_SUFFIX ""
#define T_ALARM_CURRENT_SOUND "Aktualny dźwięk budzika"
#define T_ALARM_TIME_LABEL "Godzina budzenia"
#define T_ALARM_ENABLE "Włącz budzik"
#define T_ALARM_VOLUME_LABEL "Głośność budzika"
#define T_ALARM_SAVE_BUTTON "💾 Zapisz ustawienia budzika"
#define T_ALARM_NEED_STATIONS_MAIN "Najpierw prześlij listę stacji M3U na stronie głównej."
#define T_ALARM_TEST_HEADING "Test"
#define T_ALARM_TEST_BUTTON "🔔 Test budzika"
#define T_ALARM_SAVED_MSG "✅ Ustawienia budzika zapisane!"
#define T_ALARM_TRIGGERED_MSG "🔔 Sygnał budzika uruchomiony!"
#define T_ALARM_STOP_TEST_BUTTON "⏹ Zakończ test budzika"
#define T_ALARM_STOPPED_MSG "⏹ Test budzika zakończony."
#define T_ALARM_NEED_STATIONS "Najpierw prześlij listę stacji M3U."

#define T_SD_PAGE_TITLE "MicroSD - ESP32 Radio"
#define T_SD_HEADING "Pliki na karcie microSD"
#define T_SD_INTRO "Odtwarzaj lokalne pliki audio lub zapisz je jako źródło budzika."
#define T_SD_PLAY_ALL "▶ Odtwórz wszystkie"
#define T_SD_STOP_ALL "■ Zatrzymaj wszystkie"
#define T_SD_REFRESH "↻ Odśwież"
#define T_SD_UPLOAD_HEADING "Prześlij dźwięk budzika"
#define T_SD_FILE_ARIA "Wybierz dźwięk budzika"
#define T_SD_UPLOAD_BUTTON "Prześlij dźwięk budzika"
#define T_SD_FILES_HEADING "Pliki audio"
#define T_SD_LOADING_FILES "Ładowanie plików ..."
#define T_BACK_TO_RADIO "Wróć do radia"
#define T_SD_ACTION_FAILED "Akcja nie powiodła się"
#define T_SD_PLAY_TITLE "Odtwórz na urządzeniu"
#define T_SD_PLAY_STARTED "Odtwarzanie rozpoczęte."
#define T_SD_SET_ALARM_TITLE "Zapisz jako dźwięk budzika"
#define T_SD_ALARM_SAVED "Plik zapisany jako źródło budzika."
#define T_SD_DOWNLOAD_TITLE "Pobierz"
#define T_SD_DELETE_TITLE "Usuń"
#define T_SD_DELETE_CONFIRM_PREFIX "Usunąć plik "
#define T_SD_DELETE_CONFIRM_SUFFIX "?"
#define T_SD_DELETED "Plik usunięty."
#define T_SD_LIST_UNREACHABLE "Lista SD niedostępna"
#define T_SD_NO_FILES "Nie znaleziono plików do odtworzenia lub brak karty SD."
#define T_SD_FILES_FOUND_PREFIX "Znaleziono plików: "
#define T_SD_FILES_FOUND_SUFFIX "."
#define T_SD_LOAD_FAILED "Nie udało się wczytać plików SD."
#define T_SD_UPLOAD_SIZE_HINT "Wybierz plik o rozmiarze od 1 bajta do 20 MB."
#define T_SD_UPLOAD_RUNNING "Przesyłanie..."
#define T_SD_PLAY_ALL_MSG "Wszystkie pliki będą odtwarzane po kolei."
#define T_SD_STOPPED "Odtwarzanie zatrzymane."

#define T_SETUP_PAGE_TITLE "Konfiguracja Wi-Fi - ESP32 Radio"
#define T_SETUP_HEADING "⚙️ Ustawienia Wi-Fi"
#define T_SETUP_SUBTITLE "Połącz swoje ESP32 Radio z siecią domową"
#define T_SETUP_CONNECTED_PREFIX "Połączone z "
#define T_SETUP_CONNECTED_SUFFIX ""
#define T_SETUP_NETWORKS_LABEL "Dostępne sieci (2,4 GHz)"
#define T_SETUP_SCAN_BUTTON "🔄 Skanuj"
#define T_SETUP_SCAN_HINT "Kliknij Skanuj, aby wyszukać sieci"
#define T_SETUP_MANUAL_SSID_LABEL "Lub wpisz SSID ręcznie"
#define T_SETUP_SSID_PLACEHOLDER "Nazwa sieci Wi-Fi (SSID)"
#define T_SETUP_WIFI_PASS_LABEL "Hasło Wi-Fi"
#define T_SETUP_WIFI_PASS_PLACEHOLDER "Wpisz hasło"
#define T_SETUP_SAVE_CONNECT "💾 Zapisz i połącz"
#define T_SETUP_OPTIONS_HEADING "Opcje"
#define T_SETUP_RESET_BUTTON "🗑️ Usuń zapisane dane Wi-Fi"
#define T_SETUP_MQTT_HOST_LABEL "Adres brokera"
#define T_SETUP_MQTT_USER_LABEL "Nazwa użytkownika (opcjonalnie)"
#define T_SETUP_MQTT_LEAVE_EMPTY "można zostawić puste"
#define T_SETUP_MQTT_PASS_LABEL "Hasło (opcjonalnie)"
#define T_SETUP_MQTT_BASE_LABEL "Bazowy temat MQTT"
#define T_SETUP_MQTT_SAVE "💾 Zapisz MQTT"
#define T_SETUP_MQTT_STATUS_LOADING "Ładowanie stanu MQTT..."
#define T_SETUP_SEARCHING_NETWORKS "Szukanie sieci..."
#define T_SETUP_SCANNING "Skanowanie sieci Wi-Fi..."
#define T_SETUP_SELECT_NETWORK "-- Wybierz sieć --"
#define T_SETUP_NO_NETWORKS "Nie znaleziono sieci"
#define T_SETUP_NETWORKS_FOUND_PREFIX "Znaleziono sieci: "
#define T_SETUP_NETWORKS_FOUND_SUFFIX "."
#define T_SETUP_SCAN_ERROR "Błąd podczas skanowania sieci."
#define T_SETUP_SSID_REQUIRED "Podaj nazwę sieci Wi-Fi (SSID)."
#define T_SETUP_SAVING_PREFIX "Zapisywanie i łączenie z "
#define T_SETUP_SAVING_SUFFIX "..."
#define T_SETUP_SAVED "✅ Zapisano! ESP32 łączy się ponownie..."
#define T_SETUP_SAVED_HOTSPOT_HINT "Jeśli byłeś w trybie hotspotu, połącz teraz telefon ponownie ze swoją zwykłą siecią Wi-Fi."
#define T_SETUP_SAVE_ERROR "Błąd podczas zapisywania danych."
#define T_SETUP_RESET_CONFIRM "Czy na pewno chcesz usunąć zapisane dane logowania Wi-Fi?"
#define T_SETUP_RESET_DONE "Dane Wi-Fi usunięte. Restartowanie..."
#define T_SETUP_MQTT_DISABLED "MQTT wyłączone"
#define T_SETUP_MQTT_CONNECTED_PREFIX "MQTT połączone z "
#define T_SETUP_MQTT_CONNECTED_SUFFIX ""
#define T_SETUP_MQTT_DISCONNECTED_PREFIX "Brak połączenia MQTT z "
#define T_SETUP_MQTT_DISCONNECTED_SUFFIX ""
#define T_SETUP_MQTT_UNKNOWN "Nieznany stan MQTT"
#define T_SETUP_MQTT_UNREACHABLE "Stan MQTT niedostępny"
#define T_SETUP_MQTT_SAVED "MQTT zapisane. Nawiązywanie połączenia."

#define T_IDX_OPTIONS "Opcje"
#define T_IDX_NAV_ALARM "⏰ Budzik"
#define T_IDX_NAV_WIFI "⚙️ Wi-Fi"
#define T_IDX_STOPPED "Zatrzymano"
#define T_IDX_BITRATE_LABEL "Bitrate: "
#define T_IDX_VOLUME "Głośność"
#define T_IDX_STOP_BUTTON "⏹ Stop"
#define T_IDX_PLAY_BUTTON "▶ Odtwórz"
#define T_IDX_FAVORITES_HEADING "Ulubione"
#define T_IDX_SHOW_ALL "Pokaż wszystkie stacje"
#define T_IDX_SAVE_AS_ALARM "⏰ Zapisz jako dźwięk budzika"
#define T_IDX_STATION_LIST_HEADING "Lista stacji"
#define T_IDX_SEARCH_PLACEHOLDER "Szukaj stacji..."
#define T_IDX_CUSTOM_URL_HEADING "Własny adres strumienia"
#define T_IDX_CUSTOM_PLAY "Odtwórz"
#define T_IDX_ANNOUNCE_VOLUME "📢 Głośność komunikatów (TTS)"
#define T_IDX_M3U_HEADING "Lista stacji M3U"
#define T_IDX_M3U_APPEND "Dołącz do istniejącej listy"
#define T_IDX_M3U_REPLACE "Zastąp istniejącą listę"
#define T_IDX_M3U_UPLOAD "📂 Prześlij M3U"
#define T_IDX_CLEAR_ALL "🗑️ Usuń wszystkie stacje"
#define T_IDX_NO_FAVORITES "Brak ulubionych – oznacz stacje gwiazdką ☆ na liście stacji."
#define T_IDX_LOAD_ERROR "Błąd ładowania"
#define T_IDX_NO_MATCHES "Brak wyników."
#define T_IDX_NO_STATIONS_PREFIX "Brak zapisanych stacji – prześlij listę M3U w sekcji "
#define T_IDX_NO_STATIONS_SUFFIX "."
#define T_IDX_PLAY_TITLE "Odtwórz"
#define T_IDX_FAV_REMOVE "Usuń z ulubionych"
#define T_IDX_FAV_ADD "Dodaj do ulubionych"
#define T_IDX_DELETE_TITLE "Usuń"
#define T_IDX_PAGE_OF " z "
#define T_IDX_NO_SD_LIMIT_SHORT " (bez karty SD maks. 24)"
#define T_IDX_FAV_CHANGE_FAILED "Nie udało się zmienić ulubionej: "
#define T_IDX_DELETE_CONFIRM_PREFIX "Usunąć stację "
#define T_IDX_DELETE_CONFIRM_SUFFIX "?"
#define T_IDX_STATION_DELETED "Stacja usunięta."
#define T_IDX_DELETE_FAILED "Usuwanie nie powiodło się: "
#define T_IDX_CLEAR_CONFIRM "Na pewno usunąć WSZYSTKIE stacje? Ulubione zostaną zachowane."
#define T_IDX_CLEARED "Wszystkie stacje usunięte."
#define T_IDX_M3U_SELECT_FILE "Najpierw wybierz plik M3U."
#define T_IDX_M3U_UPLOADING "Przesyłanie M3U..."
#define T_IDX_IMPORTED_PREFIX "Zaimportowano stacji: "
#define T_IDX_IMPORTED_MID ", łącznie: "
#define T_IDX_IMPORTED_SUFFIX "."
#define T_IDX_NVS_LIMIT_NOTE " Bez karty SD zapisywanych jest maks. 24 stacji."
#define T_IDX_UPLOAD_FAILED "Przesyłanie nie powiodło się: "
#define T_IDX_ALARM_STATION_SAVED "Bieżąca stacja jest teraz dźwiękiem budzika."
#define T_IDX_ALARM_STATION_FAILED "Nie udało się zapisać dźwięku budzika: "
#define T_IDX_UNKNOWN "Nieznane"
#define T_IDX_BADGE_ANNOUNCING "📢 Komunikat"
#define T_IDX_BADGE_PLAYING "Odtwarzanie"