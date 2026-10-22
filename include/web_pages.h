#pragma once
#include <Arduino.h>
#include "language.h"

// ==========================================
// 1. Radio web interface (main page)
// ==========================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang=")rawliteral" T_HTML_LANG R"rawliteral(">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32-S3 Internet Radio</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --accent: #38bdf8;
      --accent-hover: #0284c7;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --border: #334155;
      --active: #10b981;
      --danger: #ef4444;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 1.5rem; display: flex; justify-content: center; min-height: 100vh; }
    .container { max-width: 580px; width: 100%; display: flex; flex-direction: column; gap: 1.25rem; }
    .card { background: var(--card-bg); border-radius: 1rem; padding: 1.25rem; border: 1px solid var(--border); box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3); }
    .header-row { display: flex; justify-content: space-between; align-items: center; gap: 0.75rem; flex-wrap: wrap; margin-bottom: 0.25rem; }
    h1 { font-size: 1.5rem; color: var(--accent); }
    .header-row h1 { flex: 1 1 auto; min-width: 150px; }
    .header-actions { display: flex; gap: 0.4rem; align-items: center; justify-content: flex-end; flex-wrap: wrap; }
    .btn-settings { background: #334155; color: var(--text); padding: 0.4rem 0.65rem; font-size: 0.8rem; border-radius: 0.5rem; text-decoration: none; font-weight: 600; display: inline-flex; align-items: center; justify-content: center; gap: 0.25rem; white-space: nowrap; }
    .btn-settings:hover { background: #475569; }
    .options-toggle { color: var(--text-muted); font-size: 0.8rem; display: inline-flex; align-items: center; gap: 0.3rem; white-space: nowrap; }
    .advanced-card { display: none; }
    .advanced-card select { width: 100%; background: #0f172a; border: 1px solid var(--border); border-radius: 0.5rem; color: var(--text); padding: 0.6rem; font-size: 0.85rem; margin-bottom: 0.5rem; }
    .subtitle { text-align: center; font-size: 0.85rem; color: var(--text-muted); margin-bottom: 0.5rem; }
    .now-playing { display: flex; flex-direction: column; gap: 0.5rem; }
    .status-badge { display: inline-flex; align-items: center; gap: 0.4rem; font-size: 0.8rem; padding: 0.2rem 0.6rem; border-radius: 9999px; width: fit-content; background: #334155; }
    .status-badge.live { background: rgba(16, 185, 129, 0.2); color: #34d399; }
    .status-dot { width: 8px; height: 8px; border-radius: 50%; background: currentColor; }
    .station-name { font-size: 1.25rem; font-weight: 700; color: var(--text); }
    .track-title { font-size: 0.95rem; color: var(--text-muted); word-break: break-word; min-height: 1.4em; }
    .meta-tags { display: flex; gap: 0.5rem; font-size: 0.75rem; color: var(--text-muted); margin-top: 0.25rem; }
    .tag { background: #0f172a; padding: 0.2rem 0.5rem; border-radius: 0.25rem; border: 1px solid var(--border); }
    
    .controls { display: flex; flex-direction: column; gap: 1rem; }
    .btn-row { display: flex; gap: 0.75rem; }
    button { cursor: pointer; border: none; border-radius: 0.5rem; font-weight: 600; padding: 0.75rem 1rem; transition: all 0.2s; font-size: 0.95rem; }
    .btn-primary { background: var(--accent); color: #0f172a; flex: 1; }
    .btn-primary:hover { background: var(--accent-hover); }
    .btn-danger { background: var(--danger); color: white; flex: 1; }
    .btn-danger:hover { opacity: 0.9; }
    .btn-play { background: var(--active); color: #052e16; flex: 1; }
    .btn-play:hover { background: #059669; }
    
    .volume-box { display: flex; flex-direction: column; gap: 0.4rem; }
    .volume-label { display: flex; justify-content: space-between; font-size: 0.85rem; color: var(--text-muted); }
    input[type=range] { width: 100%; height: 6px; border-radius: 3px; background: #334155; outline: none; -webkit-appearance: none; accent-color: var(--accent); }
    
    .station-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(130px, 1fr)); gap: 0.5rem; }
    .btn-station { background: #0f172a; color: var(--text); border: 1px solid var(--border); padding: 0.6rem 0.5rem; font-size: 0.85rem; text-align: center; border-radius: 0.5rem; }
    .btn-station:hover { border-color: var(--accent); background: #1e293b; }
    .btn-station .genre { display: block; font-size: 0.7rem; color: var(--text-muted); font-weight: normal; margin-top: 0.2rem; }
    .alarm-station-button { display: block; width: auto; margin: 1rem auto 0; padding: 0.55rem 0.8rem; font-size: 0.82rem; }
    .hint { font-size: 0.8rem; color: var(--text-muted); }
    .station-search { width: 100%; background: #0f172a; border: 1px solid var(--border); border-radius: 0.5rem; color: var(--text); padding: 0.6rem; font-size: 0.85rem; outline: none; margin-bottom: 0.5rem; }
    .station-row { display: flex; align-items: center; gap: 0.4rem; padding: 0.35rem 0; border-bottom: 1px solid var(--border); }
    .station-row-name { flex: 1; font-size: 0.85rem; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
    .station-row button { padding: 0.35rem 0.55rem; font-size: 0.85rem; background: #0f172a; color: var(--text); border: 1px solid var(--border); }
    .station-row button:hover { border-color: var(--accent); }
    .pager { display: flex; justify-content: space-between; align-items: center; gap: 0.5rem; margin-top: 0.5rem; font-size: 0.8rem; color: var(--text-muted); }
    .pager button { padding: 0.4rem 0.8rem; font-size: 0.8rem; background: #0f172a; color: var(--text); border: 1px solid var(--border); }
    .pager button:disabled { opacity: 0.4; cursor: default; }

    .custom-url-box { display: flex; gap: 0.5rem; margin-top: 0.5rem; }
    .custom-url-box input { flex: 1; background: #0f172a; border: 1px solid var(--border); border-radius: 0.5rem; color: var(--text); padding: 0.6rem; font-size: 0.85rem; outline: none; }
    .custom-url-box input:focus { border-color: var(--accent); }
    .custom-url-box button { padding: 0.6rem 1rem; }
    
    .system-info { display: flex; justify-content: space-between; font-size: 0.75rem; color: var(--text-muted); text-align: center; }
    @media (max-width: 520px) {
      body { padding: 1rem; }
      .header-row { align-items: flex-start; }
      .header-actions { width: 100%; display: grid; grid-template-columns: repeat(3, 1fr); gap: 0.4rem; }
      .options-toggle { grid-column: 1 / -1; }
      .btn-settings { width: 100%; padding: 0.45rem 0.35rem; font-size: 0.78rem; }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <div class="header-row">
        <h1>📻 ESP32 Radio</h1>
        <div class="header-actions">
          <label class="options-toggle"><input type="checkbox" id="optionsToggle"> )rawliteral" T_IDX_OPTIONS R"rawliteral(</label>
          <a href="/alarm" class="btn-settings">)rawliteral" T_IDX_NAV_ALARM R"rawliteral(</a>
          <a href="/setup" class="btn-settings">)rawliteral" T_IDX_NAV_WIFI R"rawliteral(</a>
          <a href="/sdcard" class="btn-settings">💾 MicroSD</a>
        </div>
      </div>
      <div class="subtitle">Audio Streamer</div>
      
      <div class="now-playing">
        <div id="statusBadge" class="status-badge">
          <div class="status-dot"></div>
          <span id="statusText">)rawliteral" T_IDX_STOPPED R"rawliteral(</span>
        </div>
        <div class="station-name" id="stationName">)rawliteral" T_LOADING R"rawliteral(</div>
        <div class="track-title" id="trackTitle">--</div>
        <div class="meta-tags">
          <div class="tag">)rawliteral" T_IDX_BITRATE_LABEL R"rawliteral(<span id="bitrate">--</span></div>
          <div class="tag">I2S: BCLK=9, LRC=7, DIN=8</div>
        </div>
      </div>
    </div>

    <div class="card controls">
      <div class="volume-box">
        <div class="volume-label">
          <span>)rawliteral" T_IDX_VOLUME R"rawliteral(</span>
          <span id="volVal">12 / 21</span>
        </div>
        <input type="range" id="volSlider" min="0" max="21" value="12" oninput="setVolume(this.value)">
      </div>
      <div class="btn-row">
        <button class="btn-danger" id="playbackToggle" onclick="togglePlayback()">)rawliteral" T_IDX_STOP_BUTTON R"rawliteral(</button>
      </div>
    </div>

    <div class="card">
      <div class="header-row" style="margin-bottom: 0.75rem;">
        <h3 style="font-size: 1rem; color: var(--accent);">)rawliteral" T_IDX_FAVORITES_HEADING R"rawliteral(</h3>
        <label class="options-toggle"><input type="checkbox" id="showAllStations" aria-controls="stationBrowser"> )rawliteral" T_IDX_SHOW_ALL R"rawliteral(</label>
      </div>
      <div class="station-grid" id="presetList"></div>
      <button type="button" class="btn-primary alarm-station-button" id="saveRadioAlarm">)rawliteral" T_IDX_SAVE_AS_ALARM R"rawliteral(</button>

      <div id="stationBrowser" hidden>
      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">)rawliteral" T_IDX_STATION_LIST_HEADING R"rawliteral(</h3>
      <input type="search" id="stationSearch" class="station-search" placeholder=")rawliteral" T_IDX_SEARCH_PLACEHOLDER R"rawliteral(">
      <div id="stationList"></div>
      <div class="pager">
        <button type="button" id="stationPrev">◀</button>
        <span id="stationPageInfo">--</span>
        <button type="button" id="stationNext">▶</button>
      </div>
      </div>
      <div id="stationMsg" class="hint" style="margin-top: 0.5rem;"></div>

      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">)rawliteral" T_IDX_CUSTOM_URL_HEADING R"rawliteral(</h3>
      <div class="custom-url-box">
        <input type="text" id="customUrl" placeholder="http://.../stream.mp3">
        <button class="btn-primary" onclick="playCustomUrl()">)rawliteral" T_IDX_CUSTOM_PLAY R"rawliteral(</button>
      </div>
    </div>

    <div class="card advanced-card" id="advancedCard">
      <div class="volume-box">
        <div class="volume-label">
          <span>)rawliteral" T_IDX_ANNOUNCE_VOLUME R"rawliteral(</span>
          <span id="annVolVal">14 / 21</span>
        </div>
        <input type="range" id="annVolSlider" min="0" max="21" value="14" oninput="setAnnounceVolume(this.value)">
      </div>
      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">)rawliteral" T_IDX_M3U_HEADING R"rawliteral(</h3>
      <form id="m3uForm" enctype="multipart/form-data">
        <input type="file" id="m3uFile" name="file" accept=".m3u,.m3u8,.txt" style="width: 100%; color: var(--text-muted); margin-bottom: 0.5rem;">
        <select id="m3uMode" style="width: 100%; margin-bottom: 0.5rem;">
          <option value="append">)rawliteral" T_IDX_M3U_APPEND R"rawliteral(</option>
          <option value="replace">)rawliteral" T_IDX_M3U_REPLACE R"rawliteral(</option>
        </select>
        <button type="submit" class="btn-primary">)rawliteral" T_IDX_M3U_UPLOAD R"rawliteral(</button>
      </form>
      <div id="m3uMsg" style="font-size: 0.8rem; color: var(--text-muted); margin-top: 0.5rem;"></div>
      <button type="button" class="btn-danger" id="clearStations" style="width: 100%; margin-top: 0.75rem;">)rawliteral" T_IDX_CLEAR_ALL R"rawliteral(</button>
    </div>

    <div class="system-info">
      <span id="sysHeap">RAM: --</span>
      <span id="sysPsram">PSRAM: --</span>
      <span id="sysIp">IP: --</span>
    </div>
  </div>

  <script>
    let favorites = [];
    let radioIsPlaying = false;
    let stationPage = null;
    let stationOffset = 0;
    let stationQuery = '';
    const STATION_PAGE_SIZE = 20;
    const showAllStations = document.getElementById('showAllStations');
    const stationBrowser = document.getElementById('stationBrowser');
    showAllStations.checked = localStorage.getItem('radioShowAllStations') === 'true';
    stationBrowser.hidden = !showAllStations.checked;
    showAllStations.addEventListener('change', () => {
      localStorage.setItem('radioShowAllStations', showAllStations.checked);
      stationBrowser.hidden = !showAllStations.checked;
      if (showAllStations.checked) loadStations();
    });

    function postForm(url, fields) {
      return fetch(url, { method: 'POST', body: new URLSearchParams(fields) })
        .then(res => res.ok ? res.text() : res.text().then(text => Promise.reject(text)));
    }

    function loadFavorites() {
      return fetch('/api/favorites')
        .then(res => res.json())
        .then(data => {
          favorites = data.favorites || [];
          const list = document.getElementById('presetList');
          list.innerHTML = '';
          if (!favorites.length) {
            const hint = document.createElement('div');
            hint.className = 'hint';
            hint.innerText = ')rawliteral" T_IDX_NO_FAVORITES R"rawliteral(';
            list.appendChild(hint);
          }
          favorites.forEach(f => {
            const btn = document.createElement('button');
            btn.className = 'btn-station';
            const label = document.createElement('strong');
            label.textContent = f.name;
            btn.appendChild(label);
            btn.onclick = () => playFavorite(f.idx);
            list.appendChild(btn);
          });
          renderStationPage();
        })
        .catch(err => console.error(err));
    }

    function loadStations() {
      if (stationBrowser.hidden) return;
      const params = new URLSearchParams({ offset: stationOffset, limit: STATION_PAGE_SIZE, q: stationQuery });
      document.getElementById('stationPageInfo').innerText = ')rawliteral" T_LOADING R"rawliteral(';
      fetch('/api/stations?' + params)
        .then(res => res.json())
        .then(data => {
          stationPage = data;
          renderStationPage();
        })
        .catch(() => { document.getElementById('stationPageInfo').innerText = ')rawliteral" T_IDX_LOAD_ERROR R"rawliteral('; });
    }

    function renderStationPage() {
      if (!stationPage) return;
      const box = document.getElementById('stationList');
      box.innerHTML = '';
      const stations = stationPage.stations || [];
      if (!stations.length) {
        const hint = document.createElement('div');
        hint.className = 'hint';
        hint.innerText = stationQuery ? ')rawliteral" T_IDX_NO_MATCHES R"rawliteral(' : ')rawliteral" T_IDX_NO_STATIONS_PREFIX R"rawliteral(' + '"' + ')rawliteral" T_IDX_OPTIONS R"rawliteral(' + '"' + ')rawliteral" T_IDX_NO_STATIONS_SUFFIX R"rawliteral(';
        box.appendChild(hint);
      }
      stations.forEach(s => {
        const row = document.createElement('div');
        row.className = 'station-row';
        const name = document.createElement('span');
        name.className = 'station-row-name';
        name.textContent = s.name;
        name.title = s.url;
        const fav = favorites.find(f => f.url === s.url);
        const play = document.createElement('button');
        play.innerText = '▶';
        play.title = ')rawliteral" T_IDX_PLAY_TITLE R"rawliteral(';
        play.onclick = () => playPreset(s.id);
        const star = document.createElement('button');
        star.innerText = fav ? '★' : '☆';
        star.title = fav ? ')rawliteral" T_IDX_FAV_REMOVE R"rawliteral(' : ')rawliteral" T_IDX_FAV_ADD R"rawliteral(';
        star.onclick = () => toggleFavorite(s, fav);
        const del = document.createElement('button');
        del.innerText = '🗑';
        del.title = ')rawliteral" T_IDX_DELETE_TITLE R"rawliteral(';
        del.onclick = () => deleteStation(s);
        row.append(name, play, star, del);
        box.appendChild(row);
      });
      const total = stationPage.total || 0;
      const from = total ? stationOffset + 1 : 0;
      const to = Math.min(stationOffset + STATION_PAGE_SIZE, total);
      let info = from + '–' + to + ')rawliteral" T_IDX_PAGE_OF R"rawliteral(' + total;
      if (stationPage.storage === 'nvs') info += ')rawliteral" T_IDX_NO_SD_LIMIT_SHORT R"rawliteral(';
      document.getElementById('stationPageInfo').innerText = info;
      document.getElementById('stationPrev').disabled = stationOffset === 0;
      document.getElementById('stationNext').disabled = stationOffset + STATION_PAGE_SIZE >= total;
    }

    function toggleFavorite(station, fav) {
      const msg = document.getElementById('stationMsg');
      const request = fav ? postForm('/api/favorites/remove', { idx: fav.idx }) : postForm('/api/favorites/add', { id: station.id });
      request
        .then(() => { msg.innerText = ''; return loadFavorites(); })
        .catch(error => { msg.innerText = ')rawliteral" T_IDX_FAV_CHANGE_FAILED R"rawliteral(' + error; });
    }

    function deleteStation(station) {
      if (!confirm(')rawliteral" T_IDX_DELETE_CONFIRM_PREFIX R"rawliteral(' + '"' + station.name + '"' + ')rawliteral" T_IDX_DELETE_CONFIRM_SUFFIX R"rawliteral(')) return;
      const msg = document.getElementById('stationMsg');
      postForm('/api/stations/delete', { id: station.id })
        .then(() => { msg.innerText = ')rawliteral" T_IDX_STATION_DELETED R"rawliteral('; loadStations(); })
        .catch(error => { msg.innerText = ')rawliteral" T_IDX_DELETE_FAILED R"rawliteral(' + error; });
    }

    document.getElementById('clearStations').addEventListener('click', () => {
      if (!confirm(')rawliteral" T_IDX_CLEAR_CONFIRM R"rawliteral(')) return;
      const msg = document.getElementById('m3uMsg');
      postForm('/api/stations/clear', {})
        .then(() => {
          msg.innerText = ')rawliteral" T_IDX_CLEARED R"rawliteral(';
          stationOffset = 0;
          loadStations();
        })
        .catch(error => { msg.innerText = ')rawliteral" T_IDX_DELETE_FAILED R"rawliteral(' + error; });
    });

    let searchTimeout;
    document.getElementById('stationSearch').addEventListener('input', event => {
      clearTimeout(searchTimeout);
      searchTimeout = setTimeout(() => {
        stationQuery = event.target.value.trim();
        stationOffset = 0;
        loadStations();
      }, 400);
    });
    document.getElementById('stationPrev').addEventListener('click', () => {
      stationOffset = Math.max(0, stationOffset - STATION_PAGE_SIZE);
      loadStations();
    });
    document.getElementById('stationNext').addEventListener('click', () => {
      stationOffset += STATION_PAGE_SIZE;
      loadStations();
    });

    const optionsToggle = document.getElementById('optionsToggle');
    const advancedCard = document.getElementById('advancedCard');
    optionsToggle.checked = localStorage.getItem('radioOptions') === 'true';
    advancedCard.style.display = optionsToggle.checked ? 'block' : 'none';
    optionsToggle.addEventListener('change', () => {
      localStorage.setItem('radioOptions', optionsToggle.checked);
      advancedCard.style.display = optionsToggle.checked ? 'block' : 'none';
    });

    document.getElementById('m3uForm').addEventListener('submit', event => {
      event.preventDefault();
      const file = document.getElementById('m3uFile').files[0];
      const msg = document.getElementById('m3uMsg');
      if (!file) {
        msg.innerText = ')rawliteral" T_IDX_M3U_SELECT_FILE R"rawliteral(';
        return;
      }
      msg.innerText = ')rawliteral" T_IDX_M3U_UPLOADING R"rawliteral(';
      const data = new FormData();
      data.append('file', file);
      const mode = document.getElementById('m3uMode').value;
      fetch('/api/stations/m3u?mode=' + encodeURIComponent(mode), { method: 'POST', body: data })
        .then(res => res.ok ? res.json() : res.text().then(text => Promise.reject(text)))
        .then(result => {
          let text = ')rawliteral" T_IDX_IMPORTED_PREFIX R"rawliteral(' + result.imported + ')rawliteral" T_IDX_IMPORTED_MID R"rawliteral(' + result.total + ')rawliteral" T_IDX_IMPORTED_SUFFIX R"rawliteral(';
          if (result.storage === 'nvs') text += ')rawliteral" T_IDX_NVS_LIMIT_NOTE R"rawliteral(';
          msg.innerText = text;
          stationOffset = 0;
          loadStations();
        })
        .catch(error => { msg.innerText = ')rawliteral" T_IDX_UPLOAD_FAILED R"rawliteral(' + error; });
    });

    document.getElementById('saveRadioAlarm').addEventListener('click', () => {
      const msg = document.getElementById('stationMsg');
      fetch('/api/alarm/radio', { method: 'POST' })
        .then(res => res.ok ? res.text() : res.text().then(text => Promise.reject(text)))
        .then(() => { msg.innerText = ')rawliteral" T_IDX_ALARM_STATION_SAVED R"rawliteral('; })
        .catch(error => { msg.innerText = ')rawliteral" T_IDX_ALARM_STATION_FAILED R"rawliteral(' + error; });
    });

    function playPreset(id) {
      fetch('/api/station?id=' + id);
      setTimeout(updateStatus, 300);
    }

    function playFavorite(idx) {
      fetch('/api/favorite?idx=' + idx);
      setTimeout(updateStatus, 300);
    }

    function playCustomUrl() {
      const url = document.getElementById('customUrl').value.trim();
      if (url) {
        fetch('/api/play?url=' + encodeURIComponent(url));
        setTimeout(updateStatus, 300);
      }
    }

    function togglePlayback() {
      const button = document.getElementById('playbackToggle');
      const endpoint = radioIsPlaying ? '/api/stop' : '/api/resume';
      button.disabled = true;
      fetch(endpoint)
        .then(res => res.ok ? res.text() : res.text().then(text => Promise.reject(text)))
        .then(updateStatus)
        .catch(error => console.error(error))
        .finally(() => { button.disabled = false; });
    }

    let volTimeout;
    function setVolume(val) {
      document.getElementById('volVal').innerText = val + " / 21";
      clearTimeout(volTimeout);
      volTimeout = setTimeout(() => {
        fetch('/api/volume?val=' + val);
      }, 100);
    }

    let annVolTimeout;
    function setAnnounceVolume(val) {
      document.getElementById('annVolVal').innerText = val + " / 21";
      clearTimeout(annVolTimeout);
      annVolTimeout = setTimeout(() => {
        fetch('/api/announce/volume?val=' + val);
      }, 100);
    }

    function updateStatus() {
      fetch('/api/status')
        .then(res => res.json())
        .then(data => {
          radioIsPlaying = data.playing;
          const playbackButton = document.getElementById('playbackToggle');
          playbackButton.className = radioIsPlaying ? 'btn-danger' : 'btn-play';
          playbackButton.innerText = radioIsPlaying ? ')rawliteral" T_IDX_STOP_BUTTON R"rawliteral(' : ')rawliteral" T_IDX_PLAY_BUTTON R"rawliteral(';
          document.getElementById('stationName').innerText = data.station || ')rawliteral" T_IDX_UNKNOWN R"rawliteral(';
          document.getElementById('trackTitle').innerText = data.title || '--';
          document.getElementById('bitrate').innerText = (data.bitrate && data.bitrate !== "0") ? (Math.round(data.bitrate/1000) + " kbps") : (data.bitrate_raw || "--");
          
          const badge = document.getElementById('statusBadge');
          const badgeText = document.getElementById('statusText');
          if (data.announcing) {
            badge.className = 'status-badge live';
            badgeText.innerText = ')rawliteral" T_IDX_BADGE_ANNOUNCING R"rawliteral(';
          } else if (data.playing) {
            badge.className = 'status-badge live';
            badgeText.innerText = ')rawliteral" T_IDX_BADGE_PLAYING R"rawliteral(';
          } else {
            badge.className = 'status-badge';
            badgeText.innerText = ')rawliteral" T_IDX_STOPPED R"rawliteral(';
          }

          if (document.activeElement !== document.getElementById('volSlider')) {
            document.getElementById('volSlider').value = data.volume;
            document.getElementById('volVal').innerText = data.volume + " / 21";
          }

          if (data.announce_volume !== undefined && document.activeElement !== document.getElementById('annVolSlider')) {
            document.getElementById('annVolSlider').value = data.announce_volume;
            document.getElementById('annVolVal').innerText = data.announce_volume + " / 21";
          }

          document.getElementById('sysHeap').innerText = 'Heap: ' + Math.round(data.free_heap / 1024) + ' KB';
          document.getElementById('sysPsram').innerText = 'PSRAM: ' + Math.round(data.free_psram / 1024) + ' KB';
          document.getElementById('sysIp').innerText = 'IP: ' + data.ip;
        })
        .catch(err => console.error(err));
    }

    loadFavorites();
    loadStations();
    updateStatus();
    setInterval(updateStatus, 3000);
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 2. Wi-Fi setup web interface (configuration)
// ==========================================
const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang=")rawliteral" T_HTML_LANG R"rawliteral(">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>)rawliteral" T_SETUP_PAGE_TITLE R"rawliteral(</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --accent: #38bdf8;
      --accent-hover: #0284c7;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --border: #334155;
      --active: #10b981;
      --danger: #ef4444;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 1.5rem; display: flex; justify-content: center; min-height: 100vh; }
    .container { max-width: 480px; width: 100%; display: flex; flex-direction: column; gap: 1.25rem; }
    .card { background: var(--card-bg); border-radius: 1rem; padding: 1.5rem; border: 1px solid var(--border); box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3); }
    h1 { font-size: 1.4rem; color: var(--accent); margin-bottom: 0.25rem; text-align: center; }
    .subtitle { text-align: center; font-size: 0.85rem; color: var(--text-muted); margin-bottom: 1.25rem; }
    
    .form-group { display: flex; flex-direction: column; gap: 0.4rem; margin-bottom: 1rem; }
    label { font-size: 0.85rem; color: var(--text-muted); font-weight: 600; }
    select, input[type=text], input[type=password] { background: #0f172a; border: 1px solid var(--border); border-radius: 0.5rem; color: var(--text); padding: 0.75rem; font-size: 0.95rem; outline: none; width: 100%; }
    select:focus, input:focus { border-color: var(--accent); }
    
    .row { display: flex; gap: 0.5rem; }
    button { cursor: pointer; border: none; border-radius: 0.5rem; font-weight: 600; padding: 0.75rem 1rem; transition: all 0.2s; font-size: 0.95rem; width: 100%; }
    .btn-primary { background: var(--accent); color: #0f172a; }
    .btn-primary:hover { background: var(--accent-hover); }
    .btn-secondary { background: #334155; color: var(--text); }
    .btn-secondary:hover { background: #475569; }
    .btn-danger { background: var(--danger); color: white; margin-top: 1rem; }
    .btn-danger:hover { opacity: 0.9; }
    
    .status-msg { margin-top: 1rem; padding: 0.75rem; border-radius: 0.5rem; font-size: 0.85rem; text-align: center; display: none; }
    .status-msg.info { background: rgba(56, 189, 248, 0.15); color: var(--accent); border: 1px solid var(--accent); }
    .status-msg.success { background: rgba(16, 185, 129, 0.15); color: #34d399; border: 1px solid var(--active); }
    .status-msg.error { background: rgba(239, 68, 68, 0.15); color: #f87171; border: 1px solid var(--danger); }
    
    .spinner { display: inline-block; width: 16px; height: 16px; border: 2px solid rgba(255,255,255,.3); border-radius: 50%; border-top-color: #fff; animation: spin 1s ease-in-out infinite; vertical-align: middle; margin-right: 0.4rem; }
    @keyframes spin { to { transform: rotate(360deg); } }
    
    .nav-link { display: block; text-align: center; margin-top: 1rem; color: var(--text-muted); text-decoration: none; font-size: 1.6rem; }
    .nav-link:hover { color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>)rawliteral" T_SETUP_HEADING R"rawliteral(</h1>
      <div class="subtitle" id="wifiSubtitle">)rawliteral" T_SETUP_SUBTITLE R"rawliteral(</div>

      <div class="form-group">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.3rem;">
          <label for="ssidSelect">)rawliteral" T_SETUP_NETWORKS_LABEL R"rawliteral(</label>
          <button type="button" class="btn-secondary" onclick="scanWifi()" style="width: auto; padding: 0.3rem 0.6rem; font-size: 0.75rem;">)rawliteral" T_SETUP_SCAN_BUTTON R"rawliteral(</button>
        </div>
        <select id="ssidSelect" onchange="onSsidSelect()">
          <option value="">)rawliteral" T_SETUP_SCAN_HINT R"rawliteral(</option>
        </select>
      </div>

      <div class="form-group">
        <label for="manualSsid">)rawliteral" T_SETUP_MANUAL_SSID_LABEL R"rawliteral(</label>
        <input type="text" id="manualSsid" placeholder=")rawliteral" T_SETUP_SSID_PLACEHOLDER R"rawliteral(">
      </div>

      <div class="form-group">
        <label for="wifiPass">)rawliteral" T_SETUP_WIFI_PASS_LABEL R"rawliteral(</label>
        <input type="password" id="wifiPass" placeholder=")rawliteral" T_SETUP_WIFI_PASS_PLACEHOLDER R"rawliteral(">
      </div>

      <button class="btn-primary" onclick="saveWifi()">)rawliteral" T_SETUP_SAVE_CONNECT R"rawliteral(</button>

      <div id="statusMsg" class="status-msg"></div>

      <a href="/" class="nav-link" title=")rawliteral" T_BACK_TO_RADIO R"rawliteral(" aria-label=")rawliteral" T_BACK_TO_RADIO R"rawliteral(">&larr; 📻</a>
    </div>

    <div class="card" style="margin-top: 0.5rem;">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.5rem;">)rawliteral" T_SETUP_OPTIONS_HEADING R"rawliteral(</h3>
      <button class="btn-danger" onclick="resetWifi()">)rawliteral" T_SETUP_RESET_BUTTON R"rawliteral(</button>
    </div>

    <div class="card">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.75rem;">MQTT / Home Assistant</h3>
      <div class="form-group">
        <label for="mqttHost">)rawliteral" T_SETUP_MQTT_HOST_LABEL R"rawliteral(</label>
        <input type="text" id="mqttHost" placeholder="192.168.10.3">
      </div>
      <div class="form-group">
        <label for="mqttPort">Port</label>
        <input type="text" id="mqttPort" value="1883" inputmode="numeric">
      </div>
      <div class="form-group">
        <label for="mqttUser">)rawliteral" T_SETUP_MQTT_USER_LABEL R"rawliteral(</label>
        <input type="text" id="mqttUser" placeholder=")rawliteral" T_SETUP_MQTT_LEAVE_EMPTY R"rawliteral(">
      </div>
      <div class="form-group">
        <label for="mqttPass">)rawliteral" T_SETUP_MQTT_PASS_LABEL R"rawliteral(</label>
        <input type="password" id="mqttPass" placeholder=")rawliteral" T_SETUP_MQTT_LEAVE_EMPTY R"rawliteral(">
      </div>
      <div class="form-group">
        <label for="mqttBase">)rawliteral" T_SETUP_MQTT_BASE_LABEL R"rawliteral(</label>
        <input type="text" id="mqttBase" value="esp32radio">
      </div>
      <button class="btn-primary" onclick="saveMqtt()">)rawliteral" T_SETUP_MQTT_SAVE R"rawliteral(</button>
      <div id="mqttMsg" class="status-msg"></div>
      <div id="mqttStatus" class="status-msg info" style="display: block;">)rawliteral" T_SETUP_MQTT_STATUS_LOADING R"rawliteral(</div>
    </div>
  </div>

  <script>
    function showMsg(text, type) {
      const msg = document.getElementById('statusMsg');
      msg.className = 'status-msg ' + type;
      msg.innerHTML = text;
      msg.style.display = 'block';
    }

    function scanWifi() {
      const sel = document.getElementById('ssidSelect');
      sel.innerHTML = '<option value="">)rawliteral" T_SETUP_SEARCHING_NETWORKS R"rawliteral(</option>';
      showMsg('<span class="spinner"></span> )rawliteral" T_SETUP_SCANNING R"rawliteral(', 'info');

      fetch('/api/wifi/scan')
        .then(res => res.json())
        .then(networks => {
          sel.innerHTML = '<option value="">)rawliteral" T_SETUP_SELECT_NETWORK R"rawliteral(</option>';
          if (networks.length === 0) {
            sel.innerHTML += '<option value="">)rawliteral" T_SETUP_NO_NETWORKS R"rawliteral(</option>';
          } else {
            networks.forEach(net => {
              const opt = document.createElement('option');
              opt.value = net.ssid;
              opt.innerText = `${net.ssid} (${net.rssi} dBm)`;
              sel.appendChild(opt);
            });
          }
          showMsg(`)rawliteral" T_SETUP_NETWORKS_FOUND_PREFIX R"rawliteral(${networks.length})rawliteral" T_SETUP_NETWORKS_FOUND_SUFFIX R"rawliteral(`, 'info');
        })
        .catch(err => {
          showMsg(')rawliteral" T_SETUP_SCAN_ERROR R"rawliteral(', 'error');
        });
    }

    function onSsidSelect() {
      const val = document.getElementById('ssidSelect').value;
      if (val) {
        document.getElementById('manualSsid').value = val;
      }
    }

    function saveWifi() {
      const ssid = document.getElementById('manualSsid').value.trim();
      const pass = document.getElementById('wifiPass').value.trim();

      if (!ssid) {
        showMsg(')rawliteral" T_SETUP_SSID_REQUIRED R"rawliteral(', 'error');
        return;
      }

      showMsg('<span class="spinner"></span> )rawliteral" T_SETUP_SAVING_PREFIX R"rawliteral(' + ssid + ')rawliteral" T_SETUP_SAVING_SUFFIX R"rawliteral(', 'info');

      const formData = new FormData();
      formData.append('ssid', ssid);
      formData.append('pass', pass);

      fetch('/api/wifi/save', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(text => {
          showMsg(')rawliteral" T_SETUP_SAVED R"rawliteral(<br><br><small>)rawliteral" T_SETUP_SAVED_HOTSPOT_HINT R"rawliteral(</small>', 'success');
        })
        .catch(err => {
          showMsg(')rawliteral" T_SETUP_SAVE_ERROR R"rawliteral(', 'error');
        });
    }

    function resetWifi() {
      if (confirm(')rawliteral" T_SETUP_RESET_CONFIRM R"rawliteral(')) {
        fetch('/api/wifi/reset', { method: 'POST' })
          .then(() => {
            showMsg(')rawliteral" T_SETUP_RESET_DONE R"rawliteral(', 'info');
            setTimeout(() => { location.reload(); }, 2000);
          });
      }
    }

    function loadMqtt() {
      fetch('/api/mqtt/config')
        .then(res => res.json())
        .then(data => {
          document.getElementById('mqttHost').value = data.host || '';
          document.getElementById('mqttPort').value = data.port || 1883;
          document.getElementById('mqttUser').value = data.user || '';
          document.getElementById('mqttBase').value = data.base || 'esp32radio';
        });
    }

    function loadMqttStatus() {
      fetch('/api/mqtt/status')
        .then(res => res.json())
        .then(data => {
          const status = document.getElementById('mqttStatus');
          const labels = {
            disabled: ')rawliteral" T_SETUP_MQTT_DISABLED R"rawliteral(',
            connected: ')rawliteral" T_SETUP_MQTT_CONNECTED_PREFIX R"rawliteral(' + data.host + ':' + data.port + ')rawliteral" T_SETUP_MQTT_CONNECTED_SUFFIX R"rawliteral(',
            disconnected: ')rawliteral" T_SETUP_MQTT_DISCONNECTED_PREFIX R"rawliteral(' + data.host + ':' + data.port + ')rawliteral" T_SETUP_MQTT_DISCONNECTED_SUFFIX R"rawliteral('
          };
          status.className = 'status-msg ' + (data.status === 'connected' ? 'success' : 'info');
          status.innerText = labels[data.status] || ')rawliteral" T_SETUP_MQTT_UNKNOWN R"rawliteral(';
          status.style.display = 'block';
        })
        .catch(() => {
          const status = document.getElementById('mqttStatus');
          status.className = 'status-msg error';
          status.innerText = ')rawliteral" T_SETUP_MQTT_UNREACHABLE R"rawliteral(';
          status.style.display = 'block';
        });
    }

    function saveMqtt() {
      const formData = new FormData();
      formData.append('host', document.getElementById('mqttHost').value.trim());
      formData.append('port', document.getElementById('mqttPort').value || '1883');
      formData.append('user', document.getElementById('mqttUser').value.trim());
      formData.append('pass', document.getElementById('mqttPass').value);
      formData.append('base', document.getElementById('mqttBase').value.trim() || 'esp32radio');

      fetch('/api/mqtt/config', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(() => {
          const msg = document.getElementById('mqttMsg');
          msg.className = 'status-msg success';
          msg.innerText = ')rawliteral" T_SETUP_MQTT_SAVED R"rawliteral(';
          msg.style.display = 'block';
          loadMqttStatus();
        });
    }

    // A scan briefly interrupts the Wi-Fi connection, so scan automatically only in hotspot mode.
    // While connected, the subtitle shows the current network instead.
    fetch('/api/status')
      .then(res => res.json())
      .then(data => {
        if (data.ap_mode) {
          scanWifi();
        } else if (data.ssid) {
          document.getElementById('wifiSubtitle').innerText = ')rawliteral" T_SETUP_CONNECTED_PREFIX R"rawliteral(' + '"' + data.ssid + '"' + ')rawliteral" T_SETUP_CONNECTED_SUFFIX R"rawliteral(';
        }
      })
      .catch(() => {});
    loadMqtt();
    loadMqttStatus();
    setInterval(loadMqttStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 3. Alarm web interface
// ==========================================
const char ALARM_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang=")rawliteral" T_HTML_LANG R"rawliteral(">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>)rawliteral" T_ALARM_PAGE_TITLE R"rawliteral(</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --accent: #38bdf8;
      --accent-hover: #0284c7;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --border: #334155;
      --active: #10b981;
      --danger: #ef4444;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 1.5rem; display: flex; justify-content: center; min-height: 100vh; }
    .container { max-width: 480px; width: 100%; display: flex; flex-direction: column; gap: 1.25rem; }
    .card { background: var(--card-bg); border-radius: 1rem; padding: 1.5rem; border: 1px solid var(--border); box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3); }
    h1 { font-size: 1.4rem; color: var(--accent); margin-bottom: 0.25rem; text-align: center; }
    .subtitle { text-align: center; font-size: 0.85rem; color: var(--text-muted); margin-bottom: 1.25rem; }
    
    .clock-display { text-align: center; background: #0f172a; padding: 1rem; border-radius: 0.75rem; border: 1px solid var(--border); margin-bottom: 1.25rem; }
    .clock-time { font-size: 1.8rem; font-weight: 700; color: var(--text); font-variant-numeric: tabular-nums; }
    
    .form-group { display: flex; flex-direction: column; gap: 0.4rem; margin-bottom: 1.2rem; }
    label { font-size: 0.85rem; color: var(--text-muted); font-weight: 600; }
    input[type=time] { background: #0f172a; border: 1px solid var(--border); border-radius: 0.5rem; color: var(--text); padding: 0.75rem; font-size: 1.3rem; outline: none; width: 100%; text-align: center; font-weight: 700; }
    input[type=time]:focus { border-color: var(--accent); }
    
    .toggle-row { display: flex; justify-content: space-between; align-items: center; background: #0f172a; padding: 0.75rem 1rem; border-radius: 0.5rem; border: 1px solid var(--border); }
    .switch { position: relative; display: inline-block; width: 48px; height: 26px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #334155; transition: .3s; border-radius: 26px; }
    .slider:before { position: absolute; content: ""; height: 20px; width: 20px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }
    input:checked + .slider { background-color: var(--active); }
    input:checked + .slider:before { transform: translateX(22px); }

    .volume-box { display: flex; flex-direction: column; gap: 0.4rem; margin-top: 1rem; }
    .volume-label { display: flex; justify-content: space-between; font-size: 0.85rem; color: var(--text-muted); }
    input[type=range] { width: 100%; height: 6px; border-radius: 3px; background: #334155; outline: none; -webkit-appearance: none; accent-color: var(--accent); }

    button { cursor: pointer; border: none; border-radius: 0.5rem; font-weight: 600; padding: 0.75rem 1rem; transition: all 0.2s; font-size: 0.95rem; width: 100%; margin-top: 1rem; }
    .btn-primary { background: var(--accent); color: #0f172a; }
    .btn-primary:hover { background: var(--accent-hover); }
    .btn-secondary { background: #334155; color: var(--text); }
    .btn-secondary:hover { background: #475569; }

    .status-msg { margin-top: 1rem; padding: 0.75rem; border-radius: 0.5rem; font-size: 0.85rem; text-align: center; display: none; }
    .status-msg.success { background: rgba(16, 185, 129, 0.15); color: #34d399; border: 1px solid var(--active); }
    .status-msg.error { background: rgba(239, 68, 68, 0.15); color: #f8f8f8; border: 1px solid var(--danger); }
    
    .nav-link { display: block; text-align: center; margin-top: 1rem; color: var(--text-muted); text-decoration: none; font-size: 1.6rem; }
    .nav-link:hover { color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>)rawliteral" T_ALARM_HEADING R"rawliteral(</h1>
      <div class="subtitle">)rawliteral" T_ALARM_SUBTITLE R"rawliteral(</div>

      <div class="clock-display">
        <div style="font-size: 0.75rem; color: var(--text-muted); margin-bottom: 0.2rem;">)rawliteral" T_ALARM_CLOCK_LABEL R"rawliteral(</div>
        <div class="clock-time" id="currentTime">)rawliteral" T_LOADING R"rawliteral(</div>
      </div>

      <div class="toggle-row" style="margin-bottom: 1.2rem;">
        <span style="font-size: 0.85rem; color: var(--text-muted);">)rawliteral" T_ALARM_CURRENT_SOUND R"rawliteral(</span>
        <strong id="alarmSource" style="font-size: 0.85rem; text-align: right; max-width: 60%; overflow-wrap: anywhere;">)rawliteral" T_LOADING R"rawliteral(</strong>
      </div>

      <div class="form-group">
        <label for="alarmTime">)rawliteral" T_ALARM_TIME_LABEL R"rawliteral(</label>
        <input type="time" id="alarmTime" value="07:00">
      </div>

      <div class="toggle-row">
        <span style="font-size: 0.9rem; font-weight: 600;">)rawliteral" T_ALARM_ENABLE R"rawliteral(</span>
        <label class="switch">
          <input type="checkbox" id="alarmEnabled">
          <span class="slider"></span>
        </label>
      </div>

      <div class="volume-box">
        <div class="volume-label">
          <span>)rawliteral" T_ALARM_VOLUME_LABEL R"rawliteral(</span>
          <span id="volVal">15 / 21</span>
        </div>
        <input type="range" id="alarmVol" min="0" max="21" value="15" oninput="alarmVolumeInput(this.value)">
      </div>

      <button class="btn-primary" onclick="saveAlarm()">)rawliteral" T_ALARM_SAVE_BUTTON R"rawliteral(</button>

      <div id="statusMsg" class="status-msg"></div>
      <div id="stationMsg" class="status-msg error" style="display: none;">)rawliteral" T_ALARM_NEED_STATIONS_MAIN R"rawliteral(</div>

      <a href="/" class="nav-link" title=")rawliteral" T_BACK_TO_RADIO R"rawliteral(" aria-label=")rawliteral" T_BACK_TO_RADIO R"rawliteral(">&larr; 📻</a>
    </div>

    <div class="card">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.5rem;">)rawliteral" T_ALARM_TEST_HEADING R"rawliteral(</h3>
      <button class="btn-secondary" id="testButton" onclick="testAlarm()">)rawliteral" T_ALARM_TEST_BUTTON R"rawliteral(</button>
    </div>
  </div>

  <script>
    function showMsg(text, type) {
      const msg = document.getElementById('statusMsg');
      msg.className = 'status-msg ' + type;
      msg.innerHTML = text;
      msg.style.display = 'block';
      setTimeout(() => { msg.style.display = 'none'; }, 3000);
    }

    let alarmEditing = false;
    let alarmPlaying = false;
    let alarmVolTimeout;
    let clockSeconds = null;

    function setClockFromServer(value) {
      const match = value.match(/^(\d{2}):(\d{2}):(\d{2})/);
      if (!match) {
        clockSeconds = null;
        document.getElementById('currentTime').innerText = value;
        return;
      }
      clockSeconds = Number(match[1]) * 3600 + Number(match[2]) * 60 + Number(match[3]);
      updateClock();
    }

    function updateClock() {
      if (clockSeconds === null) return;
      const hours = String(Math.floor(clockSeconds / 3600) % 24).padStart(2, '0');
      const minutes = String(Math.floor(clockSeconds / 60) % 60).padStart(2, '0');
      const seconds = String(clockSeconds % 60).padStart(2, '0');
      document.getElementById('currentTime').innerText = `${hours}:${minutes}:${seconds})rawliteral" T_ALARM_CLOCK_SUFFIX R"rawliteral(`;
      clockSeconds = (clockSeconds + 1) % 86400;
    }

    function markAlarmEditing() {
      alarmEditing = true;
    }

    // While an alarm plays, the slider changes the playing volume live (not persisted until "save")
    function alarmVolumeInput(val) {
      document.getElementById('volVal').innerText = val + ' / 21';
      if (!alarmPlaying) return;
      clearTimeout(alarmVolTimeout);
      alarmVolTimeout = setTimeout(() => {
        fetch('/api/alarm/volume?val=' + val, { method: 'POST' });
      }, 100);
    }

    function loadAlarmStatus() {
      return fetch('/api/alarm/status')
        .then(res => res.json())
        .then(data => {
          setClockFromServer(data.current_time);
          if (!alarmEditing) {
            const h = String(data.hour).padStart(2, '0');
            const m = String(data.minute).padStart(2, '0');
            document.getElementById('alarmTime').value = `${h}:${m}`;
            document.getElementById('alarmEnabled').checked = data.enabled;
            document.getElementById('alarmVol').value = data.volume;
            document.getElementById('volVal').innerText = data.volume + " / 21";
          }
            document.getElementById('alarmSource').innerText = data.alarm_label || (data.source === 'sd' ? ')rawliteral" T_ALARM_LABEL_SD_FILE_PREFIX R"rawliteral(' + data.sd_path : 'Radio');
          const available = data.station_count > 0;
          alarmPlaying = data.alarm_playing === true;
          const testButton = document.getElementById('testButton');
          testButton.innerText = alarmPlaying ? ')rawliteral" T_ALARM_STOP_TEST_BUTTON R"rawliteral(' : ')rawliteral" T_ALARM_TEST_BUTTON R"rawliteral(';
          document.getElementById('stationMsg').style.display = available ? 'none' : 'block';
          document.getElementById('alarmEnabled').disabled = !available;
          document.getElementById('alarmVol').disabled = !available && !alarmPlaying;
          document.querySelector('button[onclick="saveAlarm()"]').disabled = !available;
          testButton.disabled = !available && !alarmPlaying;
        });
    }

    function saveAlarm() {
      if (document.getElementById('alarmEnabled').disabled) return;
      const timeVal = document.getElementById('alarmTime').value;
      if (!timeVal) return;
      const parts = timeVal.split(':');
      const hour = parseInt(parts[0], 10);
      const minute = parseInt(parts[1], 10);
      const enabled = document.getElementById('alarmEnabled').checked;
      const volume = parseInt(document.getElementById('alarmVol').value, 10);

      const formData = new FormData();
      formData.append('hour', hour);
      formData.append('minute', minute);
      formData.append('enabled', enabled ? '1' : '0');
      formData.append('volume', volume);

      fetch('/api/alarm/save', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(() => {
          alarmEditing = false;
          showMsg(')rawliteral" T_ALARM_SAVED_MSG R"rawliteral(', 'success');
          loadAlarmStatus();
        });
    }

    function testAlarm() {
      const stopping = alarmPlaying;
      fetch('/api/alarm/test', { method: 'POST' })
        .then(res => {
          if (!res.ok) throw new Error('Keine Senderliste');
          showMsg(stopping ? ')rawliteral" T_ALARM_STOPPED_MSG R"rawliteral(' : ')rawliteral" T_ALARM_TRIGGERED_MSG R"rawliteral(', 'success');
          return loadAlarmStatus();
        })
        .catch(() => showMsg(')rawliteral" T_ALARM_NEED_STATIONS R"rawliteral(', 'error'));
    }

    document.getElementById('alarmTime').addEventListener('input', markAlarmEditing);
    document.getElementById('alarmEnabled').addEventListener('change', markAlarmEditing);
    document.getElementById('alarmVol').addEventListener('input', markAlarmEditing);
    loadAlarmStatus();
    setInterval(loadAlarmStatus, 2000);
    setInterval(updateClock, 1000);
  </script>
</body>
</html>
)rawliteral";


const char SDCARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang=")rawliteral" T_HTML_LANG R"rawliteral(">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>)rawliteral" T_SD_PAGE_TITLE R"rawliteral(</title>
  <style>
    :root { --bg:#0f172a; --card:#1e293b; --line:#334155; --text:#f8fafc; --muted:#94a3b8; --accent:#38bdf8; --ok:#34d399; --bad:#f87171; }
    * { box-sizing:border-box; }
    body { margin:0; padding:1.5rem; min-height:100vh; background:var(--bg); color:var(--text); font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif; }
    main { max-width:640px; margin:auto; }
    section { background:var(--card); border:1px solid var(--line); border-radius:12px; padding:1.25rem; margin-bottom:1rem; }
    h1 { margin:0 0 .25rem; color:var(--accent); font-size:1.5rem; }
    p { color:var(--muted); margin:.25rem 0 1rem; }
    .actions { display:flex; gap:.75rem; flex-wrap:wrap; }
    button { flex:1 1 180px; min-height:44px; border:0; border-radius:7px; padding:.7rem 1rem; font-weight:700; cursor:pointer; color:#082f49; background:var(--accent); }
    button.secondary { color:var(--text); background:#334155; }
    button:hover { filter:brightness(1.12); }
    #status { display:none; padding:.7rem; margin-bottom:1rem; border-radius:7px; }
    #status.ok { display:block; color:var(--ok); border:1px solid var(--ok); background:#064e3b44; }
    #status.bad { display:block; color:var(--bad); border:1px solid var(--bad); background:#7f1d1d44; }
    .file { display:flex; align-items:center; gap:.75rem; padding:.8rem 0; border-top:1px solid var(--line); }
    .file-icon { flex:0 0 auto; font-size:1.4rem; }
    .path { flex:1; min-width:0; overflow-wrap:anywhere; font-size:.95rem; }
    .file button { flex:0 0 auto; min-height:36px; padding:.5rem .7rem; }
    .file-tools { display:flex; gap:.4rem; flex-wrap:wrap; flex:0 0 auto; }
    .file-tools a { display:inline-flex; align-items:center; justify-content:center; min-height:36px; margin:0; padding:.5rem .7rem; border-radius:7px; background:#334155; color:var(--text); }
    h2 { font-size:1rem; margin:0 0 .75rem; }
    #audioUpload { display:flex; gap:.75rem; flex-wrap:wrap; align-items:center; }
    #alarmFile { min-width:0; max-width:100%; flex:1 1 240px; }
    #uploadButton { flex:0 1 auto; }
    button:disabled { opacity:.5; cursor:default; }
    @media (max-width:520px) {
      body { padding:1rem; }
      .file { flex-wrap:wrap; }
      .path { flex:1 1 calc(100% - 3rem); }
      .file-tools { width:100%; justify-content:flex-end; }
    }
    .empty { color:var(--muted); padding:.75rem 0; }
    a { color:var(--muted); display:block; text-align:center; margin-top:1rem; text-decoration:none; }
    a:hover { color:var(--accent); }
    a.nav-link { font-size:1.6rem; }
  </style>
</head>
<body>
  <main>
    <section>
      <h1>)rawliteral" T_SD_HEADING R"rawliteral(</h1>
      <p>)rawliteral" T_SD_INTRO R"rawliteral(</p>
      <div id="status" role="status"></div>
      <div class="actions">
        <button onclick="playAll()">)rawliteral" T_SD_PLAY_ALL R"rawliteral(</button>
        <button class="secondary" onclick="stopAll()">)rawliteral" T_SD_STOP_ALL R"rawliteral(</button>
        <button class="secondary" onclick="loadFiles()">)rawliteral" T_SD_REFRESH R"rawliteral(</button>
      </div>
    </section>
    <section>
      <h2>)rawliteral" T_SD_UPLOAD_HEADING R"rawliteral(</h2>
      <form id="audioUpload" enctype="multipart/form-data">
        <input type="file" id="alarmFile" name="file" accept=".mp3,.wav,.aac,.flac" aria-label=")rawliteral" T_SD_FILE_ARIA R"rawliteral(" required>
        <button type="submit" id="uploadButton">)rawliteral" T_SD_UPLOAD_BUTTON R"rawliteral(</button>
      </form>
      <div class="empty" id="uploadStatus" role="status"></div>
    </section>
    <section>
      <h2>)rawliteral" T_SD_FILES_HEADING R"rawliteral(</h2>
      <div id="fileList"><div class="empty">)rawliteral" T_SD_LOADING_FILES R"rawliteral(</div></div>
    </section>
    <a href="/" class="nav-link" title=")rawliteral" T_BACK_TO_RADIO R"rawliteral(" aria-label=")rawliteral" T_BACK_TO_RADIO R"rawliteral(">&larr; 📻</a>
  </main>
  <script>
    const list = document.getElementById('fileList');
    const status = document.getElementById('status');
    function message(text, good) {
      status.textContent = text;
      status.className = good ? 'ok' : 'bad';
    }
    async function request(url, options) {
      const response = await fetch(url, options);
      const text = await response.text();
      if (!response.ok) throw new Error(text || ')rawliteral" T_SD_ACTION_FAILED R"rawliteral(');
      return text;
    }
    function row(file) {
      const item = document.createElement('div');
      item.className = 'file';
      const icon = document.createElement('span');
      icon.className = 'file-icon';
      icon.textContent = file.path.toLowerCase().endsWith('.mp3') ? '🎵' : '🔊';
      const path = document.createElement('span');
      path.className = 'path';
      path.textContent = file.path;
      const tools = document.createElement('div');
      tools.className = 'file-tools';
      const play = document.createElement('button');
      play.textContent = '▶';
      play.title = ')rawliteral" T_SD_PLAY_TITLE R"rawliteral(';
      play.setAttribute('aria-label', play.title);
      play.onclick = () => request('/api/sd/play?path=' + encodeURIComponent(file.path))
        .then(() => message(')rawliteral" T_SD_PLAY_STARTED R"rawliteral(', true)).catch(error => message(error.message, false));
      const alarm = document.createElement('button');
      alarm.className = 'secondary';
      alarm.textContent = '⏰';
      alarm.title = ')rawliteral" T_SD_SET_ALARM_TITLE R"rawliteral(';
      alarm.setAttribute('aria-label', alarm.title);
      alarm.onclick = () => request('/api/sd/alarm', { method:'POST', body:new URLSearchParams({ path:file.path }) })
        .then(() => message(')rawliteral" T_SD_ALARM_SAVED R"rawliteral(', true)).catch(error => message(error.message, false));
      const download = document.createElement('a');
      download.href = '/api/sd/download?path=' + encodeURIComponent(file.path);
      download.download = file.path.split('/').pop();
      download.textContent = '⬇';
      download.title = ')rawliteral" T_SD_DOWNLOAD_TITLE R"rawliteral(';
      download.setAttribute('aria-label', download.title);
      const remove = document.createElement('button');
      remove.className = 'secondary';
      remove.textContent = '🗑';
      remove.title = ')rawliteral" T_SD_DELETE_TITLE R"rawliteral(';
      remove.setAttribute('aria-label', remove.title);
      remove.onclick = async () => {
        if (!confirm(')rawliteral" T_SD_DELETE_CONFIRM_PREFIX R"rawliteral(' + '"' + file.path + '"' + ')rawliteral" T_SD_DELETE_CONFIRM_SUFFIX R"rawliteral(')) return;
        remove.disabled = true;
        try {
          await request('/api/sd/delete', { method:'POST', body:new URLSearchParams({ path:file.path }) });
          await loadFiles();
          message(')rawliteral" T_SD_DELETED R"rawliteral(', true);
        } catch (error) { message(error.message, false); }
        finally { remove.disabled = false; }
      };
      tools.append(play, alarm, download, remove);
      item.append(icon, path, tools);
      return item;
    }
    async function loadFiles() {
      try {
        const files = await fetch('/api/sd/files').then(response => {
          if (!response.ok) throw new Error(')rawliteral" T_SD_LIST_UNREACHABLE R"rawliteral(');
          return response.json();
        });
        list.replaceChildren();
        if (!files.length) { list.innerHTML = '<div class="empty">)rawliteral" T_SD_NO_FILES R"rawliteral(</div>'; return; }
        files.forEach(file => list.appendChild(row(file)));
        message(')rawliteral" T_SD_FILES_FOUND_PREFIX R"rawliteral(' + files.length + ')rawliteral" T_SD_FILES_FOUND_SUFFIX R"rawliteral(', true);
      } catch (error) { list.innerHTML = '<div class="empty">)rawliteral" T_SD_LOAD_FAILED R"rawliteral(</div>'; message(error.message, false); }
    }
    document.getElementById('audioUpload').addEventListener('submit', async event => {
      event.preventDefault();
      const input = document.getElementById('alarmFile');
      const button = document.getElementById('uploadButton');
      const feedback = document.getElementById('uploadStatus');
      const file = input.files[0];
      if (!file) return;
      if (file.size > 20 * 1024 * 1024 || !file.size) {
        feedback.textContent = ')rawliteral" T_SD_UPLOAD_SIZE_HINT R"rawliteral(';
        return;
      }
      button.disabled = true;
      input.disabled = true;
      feedback.textContent = ')rawliteral" T_SD_UPLOAD_RUNNING R"rawliteral(';
      try {
        const data = new FormData();
        data.append('file', file);
        const result = await request('/api/sd/upload', { method:'POST', body:data });
        input.value = '';
        await loadFiles();
        feedback.textContent = result;
      } catch (error) { feedback.textContent = error.message; }
      finally { button.disabled = false; input.disabled = false; }
    });
    function playAll() { request('/api/sd/play-all', { method:'POST' }).then(() => message(')rawliteral" T_SD_PLAY_ALL_MSG R"rawliteral(', true)).catch(error => message(error.message, false)); }
    function stopAll() { request('/api/sd/stop-all', { method:'POST' }).then(() => message(')rawliteral" T_SD_STOPPED R"rawliteral(', true)).catch(error => message(error.message, false)); }
    loadFiles();
  </script>
</body>
</html>
)rawliteral";

