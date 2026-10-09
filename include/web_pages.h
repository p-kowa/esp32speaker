#pragma once
#include <Arduino.h>

// ==========================================
// 1. Radio Web-Oberfläche (Hauptseite)
// ==========================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
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
    .mic-meter { height: 14px; overflow: hidden; border-radius: 7px; background: #0f172a; border: 1px solid var(--border); }
    .mic-meter-fill { width: 0%; height: 100%; background: linear-gradient(90deg, #10b981 0%, #facc15 70%, #ef4444 100%); transition: width 0.12s linear; }
    .mic-status { display: flex; justify-content: space-between; gap: 0.75rem; margin: 0.5rem 0; color: var(--text-muted); font-size: 0.85rem; }
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
          <label class="options-toggle"><input type="checkbox" id="optionsToggle"> Optionen</label>
          <a href="/alarm" class="btn-settings">⏰ Wecker</a>
          <a href="/setup" class="btn-settings">⚙️ WLAN</a>
          <a href="/sdcard" class="btn-settings">💾 MicroSD</a>
          <a href="/wakeword" class="btn-settings">🎙️ Wake Word</a>
          <a href="/tongenerator" class="btn-settings">🎵 Tongenerator</a>
        </div>
      </div>
      <div class="subtitle">Audio Streamer</div>
      
      <div class="now-playing">
        <div id="statusBadge" class="status-badge">
          <div class="status-dot"></div>
          <span id="statusText">Gestoppt</span>
        </div>
        <div class="station-name" id="stationName">Lade...</div>
        <div class="track-title" id="trackTitle">--</div>
        <div class="meta-tags">
          <div class="tag">Bitrate: <span id="bitrate">--</span></div>
          <div class="tag">I2S: BCLK=9, LRC=7, DIN=8</div>
        </div>
      </div>
    </div>

    <div class="card controls">
      <div class="volume-box">
        <div class="volume-label">
          <span>Lautstärke</span>
          <span id="volVal">12 / 21</span>
        </div>
        <input type="range" id="volSlider" min="0" max="21" value="12" oninput="setVolume(this.value)">
      </div>
      <div class="btn-row">
        <button class="btn-danger" id="playbackToggle" onclick="togglePlayback()">⏹ Stop</button>
      </div>
    </div>

    <div class="card">
      <div class="header-row" style="margin-bottom: 0.75rem;">
        <h3 style="font-size: 1rem; color: var(--accent);">Favoriten</h3>
        <label class="options-toggle"><input type="checkbox" id="showAllStations" aria-controls="stationBrowser"> Alle Sender anzeigen</label>
      </div>
      <div class="station-grid" id="presetList"></div>
      <button type="button" class="btn-primary alarm-station-button" id="saveRadioAlarm">⏰ Als Weckton speichern</button>

      <div id="stationBrowser" hidden>
      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">Senderliste</h3>
      <input type="search" id="stationSearch" class="station-search" placeholder="Sender suchen...">
      <div id="stationList"></div>
      <div class="pager">
        <button type="button" id="stationPrev">◀</button>
        <span id="stationPageInfo">--</span>
        <button type="button" id="stationNext">▶</button>
      </div>
      </div>
      <div id="stationMsg" class="hint" style="margin-top: 0.5rem;"></div>

      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">Eigene Stream-URL</h3>
      <div class="custom-url-box">
        <input type="text" id="customUrl" placeholder="http://.../stream.mp3">
        <button class="btn-primary" onclick="playCustomUrl()">Play</button>
      </div>
    </div>

    <div class="card advanced-card" id="advancedCard">
      <h3 style="font-size: 0.9rem; margin-bottom: 0.6rem; color: var(--text-muted);">Mikrofontest (ICS43434)</h3>
      <button type="button" class="btn-primary" id="micToggle">Mikrofon-Test starten</button>
      <div class="mic-status"><span id="micState">Inaktiv</span><strong id="micDbfs">-60.0 dBFS</strong></div>
      <div class="mic-meter" role="meter" aria-label="Mikrofonpegel" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><div class="mic-meter-fill" id="micMeterFill"></div></div>
      <div class="volume-box" style="margin-top: 0.75rem;">
        <div class="volume-label"><label for="micSensitivity">Anzeige-Empfindlichkeit</label><span id="micSensitivityValue">1.0×</span></div>
        <input type="range" id="micSensitivity" min="1" max="8" step="0.5" value="1">
      </div>
      <div id="micError" style="font-size: 0.8rem; color: var(--danger); margin-top: 0.4rem;"></div>

      <div class="volume-box">
        <div class="volume-label">
          <span>📢 Durchsage-Lautstärke (TTS)</span>
          <span id="annVolVal">14 / 21</span>
        </div>
        <input type="range" id="annVolSlider" min="0" max="21" value="14" oninput="setAnnounceVolume(this.value)">
      </div>
      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">M3U-Senderliste</h3>
      <form id="m3uForm" enctype="multipart/form-data">
        <input type="file" id="m3uFile" name="file" accept=".m3u,.m3u8,.txt" style="width: 100%; color: var(--text-muted); margin-bottom: 0.5rem;">
        <select id="m3uMode" style="width: 100%; margin-bottom: 0.5rem;">
          <option value="append">An bestehende Liste anhängen</option>
          <option value="replace">Bestehende Liste ersetzen</option>
        </select>
        <button type="submit" class="btn-primary">📂 M3U hochladen</button>
      </form>
      <div id="m3uMsg" style="font-size: 0.8rem; color: var(--text-muted); margin-top: 0.5rem;"></div>
      <button type="button" class="btn-danger" id="clearStations" style="width: 100%; margin-top: 0.75rem;">🗑️ Alle Sender löschen</button>
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
            hint.innerText = 'Noch keine Favoriten – in der Senderliste mit ☆ markieren.';
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
      document.getElementById('stationPageInfo').innerText = 'Lade...';
      fetch('/api/stations?' + params)
        .then(res => res.json())
        .then(data => {
          stationPage = data;
          renderStationPage();
        })
        .catch(() => { document.getElementById('stationPageInfo').innerText = 'Fehler beim Laden'; });
    }

    function renderStationPage() {
      if (!stationPage) return;
      const box = document.getElementById('stationList');
      box.innerHTML = '';
      const stations = stationPage.stations || [];
      if (!stations.length) {
        const hint = document.createElement('div');
        hint.className = 'hint';
        hint.innerText = stationQuery ? 'Keine Treffer.' : 'Keine Sender gespeichert – M3U-Liste unter "Optionen" hochladen.';
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
        play.title = 'Abspielen';
        play.onclick = () => playPreset(s.id);
        const star = document.createElement('button');
        star.innerText = fav ? '★' : '☆';
        star.title = fav ? 'Aus Favoriten entfernen' : 'Zu Favoriten hinzufügen';
        star.onclick = () => toggleFavorite(s, fav);
        const del = document.createElement('button');
        del.innerText = '🗑';
        del.title = 'Löschen';
        del.onclick = () => deleteStation(s);
        row.append(name, play, star, del);
        box.appendChild(row);
      });
      const total = stationPage.total || 0;
      const from = total ? stationOffset + 1 : 0;
      const to = Math.min(stationOffset + STATION_PAGE_SIZE, total);
      let info = from + '–' + to + ' von ' + total;
      if (stationPage.storage === 'nvs') info += ' (ohne SD-Karte max. 24)';
      document.getElementById('stationPageInfo').innerText = info;
      document.getElementById('stationPrev').disabled = stationOffset === 0;
      document.getElementById('stationNext').disabled = stationOffset + STATION_PAGE_SIZE >= total;
    }

    function toggleFavorite(station, fav) {
      const msg = document.getElementById('stationMsg');
      const request = fav ? postForm('/api/favorites/remove', { idx: fav.idx }) : postForm('/api/favorites/add', { id: station.id });
      request
        .then(() => { msg.innerText = ''; return loadFavorites(); })
        .catch(error => { msg.innerText = 'Favorit konnte nicht geändert werden: ' + error; });
    }

    function deleteStation(station) {
      if (!confirm('Sender "' + station.name + '" löschen?')) return;
      const msg = document.getElementById('stationMsg');
      postForm('/api/stations/delete', { id: station.id })
        .then(() => { msg.innerText = 'Sender gelöscht.'; loadStations(); })
        .catch(error => { msg.innerText = 'Löschen fehlgeschlagen: ' + error; });
    }

    document.getElementById('clearStations').addEventListener('click', () => {
      if (!confirm('Wirklich ALLE Sender löschen? Favoriten bleiben erhalten.')) return;
      const msg = document.getElementById('m3uMsg');
      postForm('/api/stations/clear', {})
        .then(() => {
          msg.innerText = 'Alle Sender gelöscht.';
          stationOffset = 0;
          loadStations();
        })
        .catch(error => { msg.innerText = 'Löschen fehlgeschlagen: ' + error; });
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

    const micToggle = document.getElementById('micToggle');
    const micSensitivity = document.getElementById('micSensitivity');
    let micPollTimer = null;
    micSensitivity.value = localStorage.getItem('micSensitivity') || '1';
    document.getElementById('micSensitivityValue').innerText = Number(micSensitivity.value).toFixed(1) + '×';
    micSensitivity.addEventListener('input', () => {
      localStorage.setItem('micSensitivity', micSensitivity.value);
      document.getElementById('micSensitivityValue').innerText = Number(micSensitivity.value).toFixed(1) + '×';
      pollMicStatus();
    });

    function pollMicStatus() {
      fetch('/api/mic/status')
        .then(res => res.json())
        .then(data => {
          const state = document.getElementById('micState');
          const dbfs = document.getElementById('micDbfs');
          const fill = document.getElementById('micMeterFill');
          const meter = fill.parentElement;
          const error = document.getElementById('micError');
          micToggle.innerText = data.active ? 'Mikrofon-Test stoppen' : 'Mikrofon-Test starten';
          state.innerText = data.active ? 'Empfange I2S-Audio' : 'Inaktiv';
          dbfs.innerText = Number(data.dbfs).toFixed(1) + ' dBFS';
          const baseLevel = Math.max(0, Math.min(100, (Number(data.dbfs) + 60) * (100 / 60)));
          const displayLevel = Math.min(100, baseLevel * Number(micSensitivity.value));
          fill.style.width = displayLevel + '%';
          meter.setAttribute('aria-valuenow', String(Math.round(displayLevel)));
          error.innerText = data.error ? 'I2S-Fehler: ' + data.error : '';
          if (!data.active && micPollTimer) {
            clearInterval(micPollTimer);
            micPollTimer = null;
          }
        })
        .catch(() => { document.getElementById('micState').innerText = 'Status nicht erreichbar'; });
    }

    micToggle.addEventListener('click', () => {
      const stopping = micToggle.innerText.includes('stoppen');
      micToggle.disabled = true;
      fetch(stopping ? '/api/mic/stop' : '/api/mic/start', { method: 'POST' })
        .then(res => res.ok ? res.text() : res.text().then(text => Promise.reject(text)))
        .then(() => {
          if (!stopping && !micPollTimer) micPollTimer = setInterval(pollMicStatus, 250);
          pollMicStatus();
        })
        .catch(error => { document.getElementById('micError').innerText = error; })
        .finally(() => { micToggle.disabled = false; });
    });
    pollMicStatus();

    document.getElementById('m3uForm').addEventListener('submit', event => {
      event.preventDefault();
      const file = document.getElementById('m3uFile').files[0];
      const msg = document.getElementById('m3uMsg');
      if (!file) {
        msg.innerText = 'Bitte zuerst eine M3U-Datei auswählen.';
        return;
      }
      msg.innerText = 'M3U wird hochgeladen...';
      const data = new FormData();
      data.append('file', file);
      const mode = document.getElementById('m3uMode').value;
      fetch('/api/stations/m3u?mode=' + encodeURIComponent(mode), { method: 'POST', body: data })
        .then(res => res.ok ? res.json() : res.text().then(text => Promise.reject(text)))
        .then(result => {
          let text = result.imported + ' Sender importiert, insgesamt ' + result.total + '.';
          if (result.storage === 'nvs') text += ' Ohne SD-Karte werden max. 24 Sender gespeichert.';
          msg.innerText = text;
          stationOffset = 0;
          loadStations();
        })
        .catch(error => { msg.innerText = 'Upload fehlgeschlagen: ' + error; });
    });

    document.getElementById('saveRadioAlarm').addEventListener('click', () => {
      const msg = document.getElementById('stationMsg');
      fetch('/api/alarm/radio', { method: 'POST' })
        .then(res => res.ok ? res.text() : res.text().then(text => Promise.reject(text)))
        .then(() => { msg.innerText = 'Aktueller Sender ist jetzt der Weckton.'; })
        .catch(error => { msg.innerText = 'Weckton konnte nicht gespeichert werden: ' + error; });
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
          playbackButton.innerText = radioIsPlaying ? '⏹ Stop' : '▶ Play';
          document.getElementById('stationName').innerText = data.station || 'Unbekannt';
          document.getElementById('trackTitle').innerText = data.title || '--';
          document.getElementById('bitrate').innerText = (data.bitrate && data.bitrate !== "0") ? (Math.round(data.bitrate/1000) + " kbps") : (data.bitrate_raw || "--");
          
          const badge = document.getElementById('statusBadge');
          const badgeText = document.getElementById('statusText');
          if (data.announcing) {
            badge.className = 'status-badge live';
            badgeText.innerText = '📢 Durchsage';
          } else if (data.playing) {
            badge.className = 'status-badge live';
            badgeText.innerText = 'Wiedergabe';
          } else {
            badge.className = 'status-badge';
            badgeText.innerText = 'Gestoppt';
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
// 2. WLAN Setup Web-Oberfläche (Konfiguration)
// ==========================================
const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WLAN Konfiguration - ESP32 Radio</title>
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
    
    .nav-link { display: block; text-align: center; margin-top: 1rem; color: var(--text-muted); text-decoration: none; font-size: 0.85rem; }
    .nav-link:hover { color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>⚙️ WLAN-Einstellungen</h1>
      <div class="subtitle">Verbinde dein ESP32 Radio mit dem Heimnetz</div>

      <div class="form-group">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.3rem;">
          <label for="ssidSelect">Verfügbare Netzwerke (2.4 GHz)</label>
          <button type="button" class="btn-secondary" onclick="scanWifi()" style="width: auto; padding: 0.3rem 0.6rem; font-size: 0.75rem;">🔄 Scan</button>
        </div>
        <select id="ssidSelect" onchange="onSsidSelect()">
          <option value="">Lade Netzwerke...</option>
        </select>
      </div>

      <div class="form-group">
        <label for="manualSsid">Oder SSID manuell eingeben</label>
        <input type="text" id="manualSsid" placeholder="WLAN-Name (SSID)">
      </div>

      <div class="form-group">
        <label for="wifiPass">WLAN Passwort</label>
        <input type="password" id="wifiPass" placeholder="Passwort eingeben">
      </div>

      <button class="btn-primary" onclick="saveWifi()">💾 Speichern & Verbinden</button>

      <div id="statusMsg" class="status-msg"></div>

      <a href="/" class="nav-link">← Zurück zum Radio</a>
    </div>

    <div class="card" style="margin-top: 0.5rem;">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.5rem;">Optionen</h3>
      <button class="btn-danger" onclick="resetWifi()">🗑️ Gekoppelte WLAN-Daten löschen</button>
    </div>

    <div class="card">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.75rem;">MQTT / Home Assistant</h3>
      <div class="form-group">
        <label for="mqttHost">Broker-Adresse</label>
        <input type="text" id="mqttHost" placeholder="192.168.10.3">
      </div>
      <div class="form-group">
        <label for="mqttPort">Port</label>
        <input type="text" id="mqttPort" value="1883" inputmode="numeric">
      </div>
      <div class="form-group">
        <label for="mqttUser">Benutzername (optional)</label>
        <input type="text" id="mqttUser" placeholder="leer lassen möglich">
      </div>
      <div class="form-group">
        <label for="mqttPass">Passwort (optional)</label>
        <input type="password" id="mqttPass" placeholder="leer lassen möglich">
      </div>
      <div class="form-group">
        <label for="mqttBase">MQTT Topic-Basis</label>
        <input type="text" id="mqttBase" value="esp32radio">
      </div>
      <button class="btn-primary" onclick="saveMqtt()">💾 MQTT speichern</button>
      <div id="mqttMsg" class="status-msg"></div>
      <div id="mqttStatus" class="status-msg info" style="display: block;">MQTT-Status wird geladen...</div>
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
      sel.innerHTML = '<option value="">Suche Netzwerke...</option>';
      showMsg('<span class="spinner"></span> Scanne WLANs...', 'info');

      fetch('/api/wifi/scan')
        .then(res => res.json())
        .then(networks => {
          sel.innerHTML = '<option value="">-- Netzwerk auswählen --</option>';
          if (networks.length === 0) {
            sel.innerHTML += '<option value="">Keine Netzwerke gefunden</option>';
          } else {
            networks.forEach(net => {
              const opt = document.createElement('option');
              opt.value = net.ssid;
              opt.innerText = `${net.ssid} (${net.rssi} dBm)`;
              sel.appendChild(opt);
            });
          }
          showMsg(`${networks.length} Netzwerke gefunden.`, 'info');
        })
        .catch(err => {
          showMsg('Fehler beim Scannen der Netzwerke.', 'error');
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
        showMsg('Bitte gib einen WLAN-Namen (SSID) an.', 'error');
        return;
      }

      showMsg('<span class="spinner"></span> Speichere und verbinde mit ' + ssid + '...', 'info');

      const formData = new FormData();
      formData.append('ssid', ssid);
      formData.append('pass', pass);

      fetch('/api/wifi/save', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(text => {
          showMsg('✅ Gespeichert! Der ESP32 verbindet sich jetzt neu...<br><br><small>Falls du im Hotspot-Modus warst, verbinde dein Handy jetzt wieder mit deinem normalen WLAN.</small>', 'success');
        })
        .catch(err => {
          showMsg('Fehler beim Speichern der Daten.', 'error');
        });
    }

    function resetWifi() {
      if (confirm('Möchtest du die gespeicherten WLAN-Zugangsdaten wirklich löschen?')) {
        fetch('/api/wifi/reset', { method: 'POST' })
          .then(() => {
            showMsg('WLAN-Daten gelöscht. Startet neu...', 'info');
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
            disabled: 'MQTT deaktiviert',
            connected: 'MQTT zu ' + data.host + ':' + data.port + ' verbunden',
            disconnected: 'MQTT zu ' + data.host + ':' + data.port + ' getrennt'
          };
          status.className = 'status-msg ' + (data.status === 'connected' ? 'success' : 'info');
          status.innerText = labels[data.status] || 'MQTT-Status unbekannt';
          status.style.display = 'block';
        })
        .catch(() => {
          const status = document.getElementById('mqttStatus');
          status.className = 'status-msg error';
          status.innerText = 'MQTT-Status nicht erreichbar';
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
          msg.innerText = 'MQTT gespeichert. Verbindung wird aufgebaut.';
          msg.style.display = 'block';
          loadMqttStatus();
        });
    }

    scanWifi();
    loadMqtt();
    loadMqttStatus();
    setInterval(loadMqttStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 3. Wecker Web-Oberfläche
// ==========================================
const char ALARM_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wecker - ESP32 Radio</title>
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
    
    .nav-link { display: block; text-align: center; margin-top: 1rem; color: var(--text-muted); text-decoration: none; font-size: 0.85rem; }
    .nav-link:hover { color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>⏰ Radio-Wecker</h1>
      <div class="subtitle">Echtzeit-Uhrzeit & Weckeinstellungen</div>

      <div class="clock-display">
        <div style="font-size: 0.75rem; color: var(--text-muted); margin-bottom: 0.2rem;">ESP32 Uhrzeit (NTP)</div>
        <div class="clock-time" id="currentTime">Lade...</div>
      </div>

      <div class="toggle-row" style="margin-bottom: 1.2rem;">
        <span style="font-size: 0.85rem; color: var(--text-muted);">Aktueller Weckton</span>
        <strong id="alarmSource" style="font-size: 0.85rem; text-align: right; max-width: 60%; overflow-wrap: anywhere;">Lade...</strong>
      </div>

      <div class="form-group">
        <label for="alarmTime">Weckzeit</label>
        <input type="time" id="alarmTime" value="07:00">
      </div>

      <div class="toggle-row">
        <span style="font-size: 0.9rem; font-weight: 600;">Wecker aktivieren</span>
        <label class="switch">
          <input type="checkbox" id="alarmEnabled">
          <span class="slider"></span>
        </label>
      </div>

      <div class="volume-box">
        <div class="volume-label">
          <span>Weck-Lautstärke</span>
          <span id="volVal">15 / 21</span>
        </div>
        <input type="range" id="alarmVol" min="0" max="21" value="15" oninput="document.getElementById('volVal').innerText = this.value + ' / 21'">
      </div>

      <button class="btn-primary" onclick="saveAlarm()">💾 Wecker speichern</button>

      <div id="statusMsg" class="status-msg"></div>
      <div id="stationMsg" class="status-msg error" style="display: none;">Bitte zuerst eine M3U-Senderliste auf der Hauptseite hochladen.</div>

      <a href="/" class="nav-link"><h1>📻</h1></a>
    </div>

    <div class="card">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.5rem;">Testen</h3>
      <button class="btn-secondary" onclick="testAlarm()">🔔 Wecker jetzt sofort testen</button>
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
      document.getElementById('currentTime').innerText = `${hours}:${minutes}:${seconds} Uhr`;
      clockSeconds = (clockSeconds + 1) % 86400;
    }

    function markAlarmEditing() {
      alarmEditing = true;
    }

    function loadAlarmStatus() {
      fetch('/api/alarm/status')
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
            document.getElementById('alarmSource').innerText = data.alarm_label || (data.source === 'sd' ? 'SD-Datei: ' + data.sd_path : 'Radio');
          const available = data.station_count > 0;
          document.getElementById('stationMsg').style.display = available ? 'none' : 'block';
          document.getElementById('alarmEnabled').disabled = !available;
          document.getElementById('alarmVol').disabled = !available;
          document.querySelector('button[onclick="saveAlarm()"]').disabled = !available;
          document.querySelector('button[onclick="testAlarm()"]').disabled = !available;
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
          showMsg('✅ Wecker-Einstellungen gespeichert!', 'success');
          loadAlarmStatus();
        });
    }

    function testAlarm() {
      fetch('/api/alarm/test', { method: 'POST' })
        .then(res => {
          if (!res.ok) throw new Error('Keine Senderliste');
          showMsg('🔔 Weck-Signal ausgelöst!', 'success');
        })
        .catch(() => showMsg('Bitte zuerst eine M3U-Senderliste hochladen.', 'error'));
    }

    document.getElementById('alarmTime').addEventListener('input', markAlarmEditing);
    document.getElementById('alarmEnabled').addEventListener('change', markAlarmEditing);
    document.getElementById('alarmVol').addEventListener('input', markAlarmEditing);
    loadAlarmStatus();
    setInterval(updateClock, 1000);
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 4. MicroSD Web-Oberfläche
// ==========================================
const char SDCARD_LEGACY_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MicroSD - ESP32 Radio</title>
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
    
    .nav-link { display: block; text-align: center; margin-top: 1rem; color: var(--text-muted); text-decoration: none; font-size: 0.85rem; }
    .nav-link:hover { color: var(--accent); }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>💾 MicroSD</h1>
      <div class="subtitle">Verwalten der MicroSD-Karte</div>

      <h3 style="font-size: 1rem; margin-bottom: 0.75rem; color: var(--accent);">Gespeicherte Dateien</h3>
      <div class="station-grid" id="fileList"></div>

      <button class="btn-primary" onclick="saveAlarm()">💾 Datei als Wecker speichern</button>

      <div id="statusMsg" class="status-msg"></div>

      <a href="/" class="nav-link"><h1>📻</h1></a>
    </div>

    <div class="card">
      <h3 style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 0.5rem;">Abspielen</h3>
      <button class="btn-secondary" onclick="randomPlay()">🔔 Files abspielen</button>
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
      document.getElementById('currentTime').innerText = `${hours}:${minutes}:${seconds} Uhr`;
      clockSeconds = (clockSeconds + 1) % 86400;
    }

    function markAlarmEditing() {
      alarmEditing = true;
    }

    function loadAlarmStatus() {
      fetch('/api/alarm/status')
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
          const available = data.station_count > 0;
          document.getElementById('stationMsg').style.display = available ? 'none' : 'block';
          document.getElementById('alarmEnabled').disabled = !available;
          document.getElementById('alarmVol').disabled = !available;
          document.querySelector('button[onclick="saveAlarm()"]').disabled = !available;
          document.querySelector('button[onclick="testAlarm()"]').disabled = !available;
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
          showMsg('✅ Wecker-Einstellungen gespeichert!', 'success');
          loadAlarmStatus();
        });
    }

    function testAlarm() {
      fetch('/api/alarm/test', { method: 'POST' })
        .then(res => {
          if (!res.ok) throw new Error('Keine Senderliste');
          showMsg('🔔 Weck-Signal ausgelöst!', 'success');
        })
        .catch(() => showMsg('Bitte zuerst eine M3U-Senderliste hochladen.', 'error'));
    }

    document.getElementById('alarmTime').addEventListener('input', markAlarmEditing);
    document.getElementById('alarmEnabled').addEventListener('change', markAlarmEditing);
    document.getElementById('alarmVol').addEventListener('input', markAlarmEditing);
    loadAlarmStatus();
    setInterval(updateClock, 1000);
  </script>
</body>
</html>
)rawliteral";

const char SDCARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MicroSD - ESP32 Radio</title>
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
    .empty { color:var(--muted); padding:.75rem 0; }
    a { color:var(--muted); display:block; text-align:center; margin-top:1rem; text-decoration:none; }
    a:hover { color:var(--accent); }
  </style>
</head>
<body>
  <main>
    <section>
      <h1>MicroSD-Dateien</h1>
      <p>Lokale Audiodateien abspielen oder als Weckerquelle speichern.</p>
      <div id="status" role="status"></div>
      <div class="actions">
        <button onclick="playAll()">▶ Alle abspielen</button>
        <button class="secondary" onclick="stopAll()">■ Alle stoppen</button>
        <button class="secondary" onclick="loadFiles()">↻ Aktualisieren</button>
      </div>
    </section>
    <section>
      <div id="fileList"><div class="empty">Dateien werden geladen ...</div></div>
    </section>
    <a href="/">← Zurück zum Radio</a>
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
      if (!response.ok) throw new Error(text || 'Aktion fehlgeschlagen');
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
      const play = document.createElement('button');
      play.textContent = '▶ Play';
      play.onclick = () => request('/api/sd/play?path=' + encodeURIComponent(file.path))
        .then(() => message('Wiedergabe gestartet.', true)).catch(error => message(error.message, false));
      const alarm = document.createElement('button');
      alarm.className = 'secondary';
      alarm.textContent = '⏰ Wecker';
      alarm.onclick = () => request('/api/sd/alarm', { method:'POST', body:new URLSearchParams({ path:file.path }) })
        .then(() => message('Datei als Weckerquelle gespeichert.', true)).catch(error => message(error.message, false));
      item.append(icon, path, play, alarm);
      return item;
    }
    async function loadFiles() {
      try {
        const files = await fetch('/api/sd/files').then(response => response.json());
        list.replaceChildren();
        if (!files.length) { list.innerHTML = '<div class="empty">Keine abspielbaren Dateien gefunden oder keine SD-Karte eingelegt.</div>'; return; }
        files.forEach(file => list.appendChild(row(file)));
        message(files.length + ' Datei(en) gefunden.', true);
      } catch (error) { list.innerHTML = '<div class="empty">SD-Dateien konnten nicht geladen werden.</div>'; message(error.message, false); }
    }
    function playAll() { request('/api/sd/play-all', { method:'POST' }).then(() => message('Alle Dateien werden nacheinander abgespielt.', true)).catch(error => message(error.message, false)); }
    function stopAll() { request('/api/sd/stop-all', { method:'POST' }).then(() => message('Wiedergabe gestoppt.', true)).catch(error => message(error.message, false)); }
    loadFiles();
  </script>
</body>
</html>
)rawliteral";

const char WAKEWORD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wake Word - ESP32 Radio</title>
  <style>
    :root { --bg:#0f172a; --card:#1e293b; --line:#334155; --text:#f8fafc; --muted:#94a3b8; --accent:#38bdf8; --ok:#34d399; --bad:#f87171; }
    * { box-sizing:border-box; }
    body { margin:0; padding:1.5rem; min-height:100vh; background:var(--bg); color:var(--text); font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif; }
    main { max-width:640px; margin:auto; }
    section { background:var(--card); border:1px solid var(--line); border-radius:12px; padding:1.25rem; margin-bottom:1rem; }
    h1 { margin:0 0 .25rem; color:var(--accent); font-size:1.5rem; }
    p { color:var(--muted); margin:.25rem 0 1rem; }
    code { color:var(--text); }
    .actions { display:flex; gap:.75rem; flex-wrap:wrap; }
    button { flex:1 1 180px; min-height:44px; border:0; border-radius:7px; padding:.7rem 1rem; font-weight:700; cursor:pointer; color:#082f49; background:var(--accent); }
    button.secondary { color:var(--text); background:#334155; }
    button:disabled { opacity:.5; cursor:default; }
    button:hover:not(:disabled) { filter:brightness(1.12); }
    #status { display:none; padding:.7rem; margin-bottom:1rem; border-radius:7px; }
    #status.ok { display:block; color:var(--ok); border:1px solid var(--ok); background:#064e3b44; }
    #status.bad { display:block; color:var(--bad); border:1px solid var(--bad); background:#7f1d1d44; }
    .current { font-size:1.1rem; font-weight:700; margin-bottom:1rem; }
    .model { display:flex; align-items:center; gap:.75rem; padding:.8rem 0; border-top:1px solid var(--line); }
    .model.active .name { color:var(--ok); }
    .info { flex:1; min-width:0; overflow-wrap:anywhere; }
    .name { font-weight:700; }
    .meta { color:var(--muted); font-size:.85rem; margin-top:.2rem; }
    .model button { flex:0 0 auto; min-height:36px; padding:.5rem .7rem; }
    .empty { color:var(--muted); padding:.75rem 0; }
    .meter { height:10px; background:#0b1220; border:1px solid var(--line); border-radius:6px; overflow:hidden; margin:.5rem 0 .25rem; }
    #probFill { height:100%; width:0; background:var(--accent); transition:width .2s; }
    #probFill.hit { background:var(--ok); }
    #detector, #probText { margin-bottom:.5rem; }
    a { color:var(--muted); display:block; text-align:center; margin-top:1rem; text-decoration:none; }
    a:hover { color:var(--accent); }
  </style>
</head>
<body>
  <main>
    <section>
      <h1>🎙️ Wake Word</h1>
      <p>microWakeWord-Modelle (<code>name.tflite</code> + <code>name.json</code>) im Ordner <code id="folder">/wakeword</code> der MicroSD-Karte.</p>
      <label class="options-toggle"><input type="checkbox" id="optionsWwToggle"> Optionen</label>
      <div id="status" role="status"></div>
      <div class="current">Aktiv: <span id="current">-</span></div>
      <div class="meta" id="detector">Erkennung: -</div>
      <div class="meter"><div id="probFill"></div></div>
      <div class="meta" id="probText">Wahrscheinlichkeit: -</div>
      <div class="actions">
        <button class="secondary" id="disableBtn" onclick="select('')">✕ Wake Word deaktivieren</button>
        <button class="secondary" onclick="load()">↻ Aktualisieren</button>
      </div>
    </section>
    <section>
      <div id="modelList"><div class="empty">Modelle werden geladen ...</div></div>
    </section>
    <a href="/">← Zurück zum Radio</a>
  <section>
  <div class="card advanced-card" id="advancedWwCard">
<h3 style="
    font-size: 0.9rem;
    margin-top: 1rem;
    margin-bottom: 0.6rem;
    color: var(--text-muted);">
    🎤 Wake Word Erkennung
</h3>

<div class="volume-box">
    <div class="volume-label">
        <label for="probabilityCutoff">Schwellwert</label>
        <span id="probabilityCutoffValue">0.50</span>
    </div>
    <input
        type="range"
        id="probabilityCutoff"
        min="0"
        max="1"
        step="0.05"
        value="0.5">
    <small style="color: var(--text-muted);">
        Je höher, desto sicherer muss das Wake Word erkannt werden.
    </small>
</div>

<div class="volume-box" style="margin-top:0.75rem;">
    <div class="volume-label">
        <label for="slidingWindowSize">Fenstergröße</label>
        <span id="slidingWindowSizeValue">5</span>
    </div>
    <input
        type="range"
        id="slidingWindowSize"
        min="1"
        max="10"
        step="1"
        value="5">
    <small style="color: var(--text-muted);">
        Anzahl der Auswertungen für die Mittelung.
    </small>
</div>

  <div class="volume-box" style="margin-top:0.75rem;">
    <div class="volume-label">
      <label for="endOfSpeechSilence">Stille bis Satzende</label>
      <span id="endOfSpeechSilenceValue">1.000 ms</span>
    </div>
    <input
      type="range"
      id="endOfSpeechSilence"
      min="0"
      max="3000"
      step="100"
      value="1000">
    <small style="color: var(--text-muted);">
      Stillezeit nach dem Sprechen, bevor die Aufnahme endet.
    </small>
  </div>

<button
    type="button"
    class="btn-primary"
    id="saveMicroConfig"
    style="width:100%; margin-top:1rem;">
    💾 Wake Word Einstellungen speichern
</button>

<div id="microConfigMsg"
     style="font-size:0.8rem; color:var(--text-muted); margin-top:0.4rem;">
</div>
    </div>
  </section>
  </main>
  <script>
    const list = document.getElementById('modelList');
    const status = document.getElementById('status');
    function message(text, good) {
      status.textContent = text;
      status.className = good ? 'ok' : 'bad';
    }
    function empty(text) {
      const div = document.createElement('div');
      div.className = 'empty';
      div.textContent = text;
      list.replaceChildren(div);
    }
    function row(model, selected) {
      const item = document.createElement('div');
      item.className = 'model' + (model.name === selected ? ' active' : '');
      const info = document.createElement('div');
      info.className = 'info';
      const name = document.createElement('div');
      name.className = 'name';
      name.textContent = (model.wake_word || model.name) + (model.name === selected ? ' ✓' : '');
      const meta = document.createElement('div');
      meta.className = 'meta';
      const parts = [model.name + '.tflite', Math.round(model.size / 1024) + ' KB'];
      if (model.manifest) {
        if (model.cutoff) parts.push('Schwelle ' + model.cutoff);
        if (model.window) parts.push('Fenster ' + model.window);
      } else {
        parts.push('⚠ ' + model.name + '.json fehlt');
      }
      meta.textContent = parts.join(' · ');
      info.append(name, meta);
      const use = document.createElement('button');
      use.textContent = model.name === selected ? 'Aktiv' : 'Verwenden';
      use.disabled = model.name === selected || !model.manifest;
      use.onclick = () => select(model.name);
      item.append(info, use);
      return item;
    }
    async function load() {
      try {
        const data = await fetch('/api/wakeword/list').then(response => response.json());
        document.getElementById('folder').textContent = data.path;
        document.getElementById('current').textContent = data.selected || 'keins';
        document.getElementById('disableBtn').disabled = !data.selected;
        endOfSpeechSilence.value = data.end_of_speech_silence_ms ?? 1000;
        updateWakeWordStatus();
        const selectedModel = data.models.find(model => model.name === data.selected);
        if (selectedModel) {
          if (selectedModel.cutoff !== '') probabilityCutoff.value = selectedModel.cutoff;
          if (selectedModel.window !== '') slidingWindowSize.value = selectedModel.window;
          updateWakeWordStatus();
        }
        if (!data.sd) { empty('Keine MicroSD-Karte eingelegt.'); message('MicroSD-Karte nicht verfügbar.', false); return; }
        if (!data.folder) { empty('Ordner ' + data.path + ' konnte nicht angelegt werden.'); message('Ordnerfehler auf der MicroSD-Karte.', false); return; }
        if (!data.models.length) { empty('Keine Modelle gefunden. Kopiere name.tflite und name.json nach ' + data.path + '.'); status.className = ''; return; }
        list.replaceChildren(...data.models.map(model => row(model, data.selected)));
        status.className = '';
      } catch (error) { empty('Modelle konnten nicht geladen werden.'); message(error.message, false); }
    }
    async function select(name) {
      try {
        const response = await fetch('/api/wakeword/select', { method:'POST', body:new URLSearchParams({ name }) });
        const text = await response.text();
        if (!response.ok) throw new Error(text || 'Auswahl fehlgeschlagen');
        message(name ? 'Wake Word "' + name + '" gespeichert.' : 'Wake Word deaktiviert.', true);
        load();
      } catch (error) { message(error.message, false); }
    }
    let lastDetections = null;
    async function pollStatus() {
      try {
        const s = await fetch('/api/wakeword/status').then(response => response.json());
        const detector = document.getElementById('detector');
        detector.textContent = 'Erkennung: ' + (s.running ? 'läuft' : 'aus') + (s.error ? ' – ' + s.error : '') + ' · erkannt: ' + s.detections;
        const fill = document.getElementById('probFill');
        fill.style.width = Math.round(s.peak * 100) + '%';
        document.getElementById('probText').textContent = 'Wahrscheinlichkeit: ' + s.peak.toFixed(2);
        if (lastDetections !== null && s.detections > lastDetections) {
          fill.classList.add('hit');
          message('Wake Word erkannt!', true);
          setTimeout(() => fill.classList.remove('hit'), 1500);
        }
        lastDetections = s.detections;
      } catch (error) { document.getElementById('detector').textContent = 'Erkennung: Status nicht erreichbar'; }
    }
    setInterval(pollStatus, 500);
	const optionsToggle =
    document.getElementById('optionsWwToggle');

const advancedCard =
    document.getElementById('advancedWwCard');

optionsToggle.checked =
    localStorage.getItem('wakewordOptions') === 'true';

advancedCard.style.display =
    optionsToggle.checked ? 'block' : 'none';

optionsToggle.addEventListener('change', () => {

    localStorage.setItem(
        'wakewordOptions',
        optionsToggle.checked
    );

    advancedCard.style.display =
        optionsToggle.checked ? 'block' : 'none';
});

const probabilityCutoff =
    document.getElementById('probabilityCutoff');

const probabilityCutoffValue =
    document.getElementById('probabilityCutoffValue');

const slidingWindowSize =
    document.getElementById('slidingWindowSize');

const slidingWindowSizeValue =
    document.getElementById('slidingWindowSizeValue');
const endOfSpeechSilence =
  document.getElementById('endOfSpeechSilence');
const endOfSpeechSilenceValue =
  document.getElementById('endOfSpeechSilenceValue');

function updateWakeWordStatus() {

    probabilityCutoffValue.textContent =
        Number(probabilityCutoff.value).toFixed(2);

    slidingWindowSizeValue.textContent =
        slidingWindowSize.value;
    endOfSpeechSilenceValue.textContent =
      Number(endOfSpeechSilence.value).toLocaleString('de-DE') + ' ms';
}

probabilityCutoff.addEventListener(
    'input',
    updateWakeWordStatus
);

slidingWindowSize.addEventListener(
    'input',
    updateWakeWordStatus
);
endOfSpeechSilence.addEventListener(
  'input',
  updateWakeWordStatus
);

updateWakeWordStatus();

document
    .getElementById('saveMicroConfig')
    .addEventListener('click', async () => {

        const payload = {
            probability_cutoff:
            probabilityCutoff.value,

            sliding_window_size:
            slidingWindowSize.value,

            end_of_speech_silence_ms:
            endOfSpeechSilence.value
        };

        try {

            const response =
            await fetch('/api/wakeword/settings', {
                    method: 'POST',
              body: new URLSearchParams(payload)
                });

          const result = await response.text();
          document.getElementById('microConfigMsg').textContent = response.ok
            ? 'Einstellungen gespeichert und Erkennung neu gestartet.'
            : 'Speichern fehlgeschlagen: ' + result;
          if (response.ok) load();

        } catch {

            document
                .getElementById('microConfigMsg')
                .textContent =
                    '❌ Server nicht erreichbar';
        }
});
    load();
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 4. MicroSD Web-Oberfläche
// ==========================================
const char TONGENERATOR_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de" class="dark">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sequenz-Ton-Generator</title>
  <!-- Tailwind CSS -->
  <script src="https://cdn.tailwindcss.com"></script>
  <!-- Lucide Icons -->
  <script src="https://unpkg.com/lucide@latest"></script>
  <!-- Google Font -->
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Fira+Code:wght@400;600&family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  
  <script>
    tailwind.config = {
      darkMode: 'class',
      theme: {
        extend: {
          fontFamily: {
            sans: ['Inter', 'sans-serif'],
            mono: ['Fira Code', 'monospace'],
          },
          colors: {
            brand: {
              50: '#f0f9ff',
              500: '#06b6d4',
              600: '#0891b2',
              900: '#083344',
            },
            accent: {
              purple: '#a855f7',
              pink: '#ec4899',
              cyan: '#06b6d4',
              green: '#10b981'
            }
          }
        }
      }
    }
  </script>

  <style>
    body {
      background-color: #0b0f17;
      color: #f3f4f6;
    }
    .neon-border {
      box-shadow: 0 0 15px rgba(6, 182, 212, 0.15);
    }
    .neon-border:focus-within {
      box-shadow: 0 0 20px rgba(6, 182, 212, 0.35);
    }
    .active-row {
      background-color: rgba(6, 182, 212, 0.15) !important;
      border-left: 4px solid #06b6d4 !important;
    }
    /* Custom scrollbar */
    ::-webkit-scrollbar {
      width: 8px;
      height: 8px;
    }
    ::-webkit-scrollbar-track {
      background: #111827;
    }
    ::-webkit-scrollbar-thumb {
      background: #374151;
      border-radius: 4px;
    }
    ::-webkit-scrollbar-thumb:hover {
      background: #4b5563;
    }
  </style>
</head>
<body class="min-h-screen flex flex-col font-sans antialiased bg-[#0b0f17] text-gray-100">

  <!-- Header -->
  <header class="border-b border-gray-800 bg-[#111827]/80 backdrop-blur sticky top-0 z-50">
    <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 h-16 flex items-center justify-between">
      <div class="flex items-center space-x-3">
        <div class="p-2 bg-cyan-500/10 border border-cyan-500/30 rounded-lg text-cyan-400">
          <i data-lucide="audio-wave" class="w-6 h-6"></i>
        </div>
        <div>
          <h1 class="font-bold text-lg tracking-tight bg-gradient-to-r from-cyan-400 via-teal-300 to-indigo-400 bg-clip-text text-transparent">
            Sequenz-Ton-Generator
          </h1>
          <p class="text-xs text-gray-400 hidden sm:block">Audio-Frequenz-Sequenzer & Web-Audio Synthesizer</p>
        </div>
      </div>

      <!-- Quick Preset Selector -->
      <div class="flex items-center space-x-2">
        <span class="text-xs text-gray-400 hidden md:inline">Presets:</span>
        <select id="presetSelect" onchange="loadPreset(this.value)" class="bg-gray-800 border border-gray-700 text-xs text-gray-200 rounded-lg px-3 py-1.5 focus:outline-none focus:border-cyan-500 cursor-pointer">
          <option value="user_default">Dein Dreiklang (800/1100/1600)</option>
          <option value="r2d2">R2-D2 Droid Beep</option>
          <option value="jump">8-Bit Jump Sound</option>
          <option value="chime">Erfolgs-Chime</option>
          <option value="arpeggio">Moll Arpeggio</option>
          <option value="alarm">Warn-Signal</option>
        </select>
      </div>
    </div>
  </header>

  <!-- Main Content Layout -->
  <main class="flex-1 max-w-7xl w-full mx-auto px-4 sm:px-6 lg:px-8 py-6 grid grid-cols-1 lg:grid-cols-12 gap-6">

    <!-- Left Column: Controls & Visualizer + Sequence Editor (Cols 7) -->
    <section class="lg:col-span-7 flex flex-col space-y-6">
      
      <!-- Visualizer & Master Control Box -->
      <div class="bg-gray-900/90 border border-gray-800 rounded-2xl p-5 shadow-xl backdrop-blur relative overflow-hidden">
        <!-- Canvas Waveform Visualizer -->
        <div class="relative w-full h-36 bg-gray-950 rounded-xl border border-gray-800/80 overflow-hidden mb-5">
          <canvas id="visualizerCanvas" class="w-full h-full block"></canvas>
          <div class="absolute top-2 left-3 text-[10px] font-mono text-cyan-400/70 uppercase tracking-wider flex items-center gap-1.5">
            <span class="w-2 h-2 rounded-full bg-cyan-400 animate-pulse"></span> Oszilloskop Signal-Anzeige
          </div>
          <div id="playingBadge" class="hidden absolute top-2 right-3 text-[11px] font-mono bg-cyan-500/20 text-cyan-300 border border-cyan-500/40 px-2 py-0.5 rounded-full items-center gap-1">
            <i data-lucide="play" class="w-3 h-3 fill-current"></i> Wiedergabe aktiv
          </div>
        </div>

        <!-- Master Playback Controls -->
        <div class="flex flex-wrap items-center justify-between gap-4">
          <div class="flex items-center space-x-3">
            <button id="playBtn" onclick="togglePlaySequence()" class="flex items-center space-x-2 bg-gradient-to-r from-cyan-500 to-teal-500 hover:from-cyan-400 hover:to-teal-400 text-gray-950 font-bold px-5 py-2.5 rounded-xl transition-all shadow-lg shadow-cyan-500/20 active:scale-95">
              <i data-lucide="play" class="w-5 h-5 fill-current"></i>
              <span id="playBtnText">Sequenz Abspielen</span>
            </button>

            <button id="loopBtn" onclick="toggleLoop()" class="flex items-center space-x-1.5 bg-gray-800 hover:bg-gray-700 text-gray-300 border border-gray-700 px-3 py-2.5 rounded-xl text-sm font-medium transition-all">
              <i data-lucide="repeat" class="w-4 h-4"></i>
              <span id="loopLabel">Endlosschleife: Aus</span>
            </button>
          </div>

          <!-- Volume Slider -->
          <div class="flex items-center space-x-3 bg-gray-950/60 border border-gray-800 px-3 py-2 rounded-xl">
            <i data-lucide="volume-2" class="w-4 h-4 text-gray-400"></i>
            <input type="range" id="masterVolume" min="0" max="1" step="0.01" value="0.7" class="w-24 h-1.5 bg-gray-700 rounded-lg appearance-none cursor-pointer accent-cyan-400" oninput="updateVolume(this.value)">
            <span id="volValue" class="text-xs font-mono text-gray-400 w-8">70%</span>
          </div>
        </div>
      </div>

      <!-- Tone Sequence Editor Table -->
      <div class="bg-gray-900/90 border border-gray-800 rounded-2xl p-5 shadow-xl flex-1 flex flex-col">
        <div class="flex items-center justify-between mb-4">
          <div>
            <h2 class="text-base font-semibold text-gray-100 flex items-center gap-2">
              <i data-lucide="list-music" class="w-5 h-5 text-cyan-400"></i> Ton-Sequenz Schritte
            </h2>
            <p class="text-xs text-gray-400">Frequenzen (Hz) & Dauern (ms) verwalten</p>
          </div>
          
          <div class="flex items-center space-x-2">
            <button onclick="addToneRow()" class="flex items-center space-x-1 text-xs bg-cyan-500/10 hover:bg-cyan-500/20 text-cyan-400 border border-cyan-500/30 font-medium px-3 py-1.5 rounded-lg transition-all">
              <i data-lucide="plus" class="w-4 h-4"></i>
              <span>Ton hinzufügen</span>
            </button>
            <button onclick="clearSequence()" class="text-xs bg-red-500/10 hover:bg-red-500/20 text-red-400 border border-red-500/20 font-medium px-3 py-1.5 rounded-lg transition-all">
              Leeren
            </button>
          </div>
        </div>

        <!-- Scrollable Steps Table -->
        <div class="overflow-x-auto rounded-xl border border-gray-800 bg-gray-950/40 flex-1 min-h-[250px] max-h-[420px]">
          <table class="w-full text-left text-xs">
            <thead class="bg-gray-900/90 text-gray-400 uppercase font-mono tracking-wider text-[11px] border-b border-gray-800 sticky top-0 z-10">
              <tr>
                <th class="py-3 px-3 w-10 text-center">#</th>
                <th class="py-3 px-3">Frequenz (Hz)</th>
                <th class="py-3 px-3">Dauer (ms)</th>
                <th class="py-3 px-3">Wellenform</th>
                <th class="py-3 px-3 text-center">Test</th>
                <th class="py-3 px-3 text-right">Aktionen</th>
              </tr>
            </thead>
            <tbody id="sequenceTableBody" class="divide-y divide-gray-800/60 font-mono">
              <!-- Rows inserted dynamically via JS -->
            </tbody>
          </table>
        </div>

        <!-- Total Stats Footer -->
        <div class="mt-4 pt-3 border-t border-gray-800 flex justify-between items-center text-xs text-gray-400 font-mono">
          <span>Töne gesamt: <strong id="totalTonesCount" class="text-cyan-400">3</strong></span>
          <span>Gesamtdauer: <strong id="totalDurationCount" class="text-cyan-400">360 ms</strong></span>
        </div>
      </div>

    </section>

    <!-- Right Column: Code Generator & Export Tools (Cols 5) -->
    <section class="lg:col-span-5 flex flex-col space-y-6">

      <!-- Code Export Box -->
      <div class="bg-gray-900/90 border border-gray-800 rounded-2xl p-5 shadow-xl flex-1 flex flex-col">
        <div class="flex items-center justify-between mb-3">
          <h2 class="text-base font-semibold text-gray-100 flex items-center gap-2">
            <i data-lucide="code" class="w-5 h-5 text-purple-400"></i> Code-Generierung
          </h2>

          <!-- Tab buttons for Code Languages -->
          <div class="flex bg-gray-950 border border-gray-800 p-0.5 rounded-lg text-[11px]">
            <button id="tabJs" onclick="switchCodeTab('js')" class="px-2.5 py-1 rounded-md font-medium text-cyan-400 bg-gray-800 shadow">JavaScript</button>
            <button id="tabArduino" onclick="switchCodeTab('arduino')" class="px-2.5 py-1 rounded-md font-medium text-gray-400 hover:text-gray-200">Arduino / C++</button>
            <button id="tabJson" onclick="switchCodeTab('json')" class="px-2.5 py-1 rounded-md font-medium text-gray-400 hover:text-gray-200">JSON</button>
          </div>
        </div>

        <p class="text-xs text-gray-400 mb-3">
          Generierter Code zum direkten Einbauen in deine eigenen Projekte:
        </p>

        <!-- Code Snippet Display -->
        <div class="relative flex-1 bg-gray-950 border border-gray-800 rounded-xl p-3 font-mono text-xs text-cyan-300 overflow-hidden flex flex-col min-h-[260px]">
          <pre id="codeOutput" class="overflow-auto flex-1 text-[11px] leading-relaxed select-all text-gray-300"></pre>

          <button onclick="copyCodeToClipboard()" class="absolute top-2.5 right-2.5 bg-gray-800/90 hover:bg-gray-700 text-gray-300 border border-gray-700 rounded-lg px-2.5 py-1.5 text-[11px] flex items-center gap-1 transition-all shadow">
            <i data-lucide="copy" class="w-3.5 h-3.5"></i>
            <span id="copyBtnText">Kopieren</span>
          </button>
        </div>

        <!-- Import / Export Controls -->
        <div class="mt-4 pt-3 border-t border-gray-800 flex items-center justify-between gap-2">
          <button onclick="downloadJson()" class="flex-1 flex items-center justify-center space-x-1.5 bg-gray-800 hover:bg-gray-700 border border-gray-700 text-gray-300 text-xs py-2 rounded-xl transition-all">
            <i data-lucide="download" class="w-3.5 h-3.5"></i>
            <span>JSON Download</span>
          </button>

          <label class="flex-1 flex items-center justify-center space-x-1.5 bg-gray-800 hover:bg-gray-700 border border-gray-700 text-gray-300 text-xs py-2 rounded-xl transition-all cursor-pointer">
            <i data-lucide="upload" class="w-3.5 h-3.5"></i>
            <span>JSON Laden</span>
            <input type="file" id="jsonInput" accept=".json" onchange="importJson(event)" class="hidden">
          </label>
        </div>
      </div>

      <!-- User Guide Card -->
      <div class="bg-gradient-to-br from-cyan-950/30 to-purple-950/20 border border-cyan-500/20 rounded-2xl p-4 text-xs text-gray-300 space-y-2">
        <h3 class="font-semibold text-cyan-300 flex items-center gap-1.5">
          <i data-lucide="info" class="w-4 h-4"></i> Wie funktioniert der Synthesizer?
        </h3>
        <p class="text-gray-400 leading-relaxed">
          Dieses Tool nutzt die native <strong>Web Audio API</strong> deines Browsers. Jeder Ton wird durch einen virtuellen Oszillator erzeugt. Die Frequenz definiert die Tonhöhe in Hertz (Hz), und die Dauer bestimmt das Timing in Millisekunden (ms).
        </p>
      </div>

    </section>
  </main>

  <!-- Notification Toast Container -->
  <div id="toastContainer" class="fixed bottom-4 right-4 z-50 flex flex-col space-y-2 pointer-events-none"></div>

  <!-- JavaScript Application Logic -->
  <script>
    // Default 3-tone sequence as requested by user
    let sequence = [
      { freq: 800, duration: 80, type: 'sine' },
      { freq: 1100, duration: 100, type: 'sine' },
      { freq: 1600, duration: 180, type: 'sine' }
    ];

    let audioCtx = null;
    let isPlaying = false;
    let isLooping = false;
    let masterGainNode = null;
    let activeOscillators = [];
    let currentPlaybackTimeout = null;
    let activeTab = 'js'; // 'js', 'arduino', 'json'
    let activeRowIndex = -1;

    // Presets catalog
    const PRESETS = {
      user_default: [
        { freq: 800, duration: 80, type: 'sine' },
        { freq: 1100, duration: 100, type: 'sine' },
        { freq: 1600, duration: 180, type: 'sine' }
      ],
      r2d2: [
        { freq: 1500, duration: 60, type: 'sine' },
        { freq: 2200, duration: 80, type: 'sine' },
        { freq: 1200, duration: 50, type: 'triangle' },
        { freq: 2800, duration: 120, type: 'sine' },
        { freq: 1800, duration: 90, type: 'sine' }
      ],
      jump: [
        { freq: 150, duration: 40, type: 'square' },
        { freq: 300, duration: 40, type: 'square' },
        { freq: 600, duration: 60, type: 'square' },
        { freq: 1200, duration: 120, type: 'square' }
      ],
      chime: [
        { freq: 523, duration: 100, type: 'sine' }, // C5
        { freq: 659, duration: 100, type: 'sine' }, // E5
        { freq: 784, duration: 100, type: 'sine' }, // G5
        { freq: 1046, duration: 250, type: 'sine' } // C6
      ],
      arpeggio: [
        { freq: 440, duration: 90, type: 'triangle' }, // A4
        { freq: 523, duration: 90, type: 'triangle' }, // C5
        { freq: 659, duration: 90, type: 'triangle' }, // E5
        { freq: 880, duration: 180, type: 'triangle' } // A5
      ],
      alarm: [
        { freq: 880, duration: 100, type: 'sawtooth' },
        { freq: 440, duration: 100, type: 'sawtooth' },
        { freq: 880, duration: 100, type: 'sawtooth' },
        { freq: 440, duration: 100, type: 'sawtooth' }
      ]
    };

    let analyserNode = null;
    let canvas, canvasCtx;

    window.onload = function() {
      lucide.createIcons();
      initCanvas();
      renderSequenceTable();
      updateCodeOutput();
      
      // Resize listener for visualizer
      window.addEventListener('resize', resizeCanvas);
    };

    function initAudioContext() {
      if (!audioCtx) {
        const AudioContext = window.AudioContext || window.webkitAudioContext;
        audioCtx = new AudioContext();

        analyserNode = audioCtx.createAnalyser();
        analyserNode.fftSize = 2048;

        masterGainNode = audioCtx.createGain();
        const volVal = parseFloat(document.getElementById('masterVolume').value);
        masterGainNode.gain.setValueAtTime(volVal, audioCtx.currentTime);

        masterGainNode.connect(analyserNode);
        analyserNode.connect(audioCtx.destination);
      }
      if (audioCtx.state === 'suspended') {
        audioCtx.resume();
      }
    }

    function initCanvas() {
      canvas = document.getElementById('visualizerCanvas');
      canvasCtx = canvas.getContext('2d');
      resizeCanvas();
      drawVisualizer();
    }

    function resizeCanvas() {
      if (!canvas) return;
      canvas.width = canvas.parentElement.clientWidth * window.devicePixelRatio || 300;
      canvas.height = canvas.parentElement.clientHeight * window.devicePixelRatio || 150;
    }

    // Live Oscilloscope Waveform Animation
    function drawVisualizer() {
      requestAnimationFrame(drawVisualizer);
      if (!canvasCtx || !canvas) return;

      const width = canvas.width;
      const height = canvas.height;

      canvasCtx.fillStyle = '#030712';
      canvasCtx.fillRect(0, 0, width, height);

      // Grid overlay
      canvasCtx.strokeStyle = '#111827';
      canvasCtx.lineWidth = 1;
      for (let x = 0; x < width; x += 40) {
        canvasCtx.beginPath();
        canvasCtx.moveTo(x, 0);
        canvasCtx.lineTo(x, height);
        canvasCtx.stroke();
      }
      for (let y = 0; y < height; y += 30) {
        canvasCtx.beginPath();
        canvasCtx.moveTo(0, y);
        canvasCtx.lineTo(width, y);
        canvasCtx.stroke();
      }

      if (!analyserNode || !isPlaying) {
        // Draw flat line when idle
        canvasCtx.beginPath();
        canvasCtx.strokeStyle = '#0891b2';
        canvasCtx.lineWidth = 2;
        canvasCtx.moveTo(0, height / 2);
        canvasCtx.lineTo(width, height / 2);
        canvasCtx.stroke();
        return;
      }

      const bufferLength = analyserNode.frequencyBinCount;
      const dataArray = new Uint8Array(bufferLength);
      analyserNode.getByteTimeDomainData(dataArray);

      canvasCtx.lineWidth = 2.5;
      canvasCtx.strokeStyle = '#06b6d4';
      canvasCtx.shadowBlur = 8;
      canvasCtx.shadowColor = '#06b6d4';
      canvasCtx.beginPath();

      const sliceWidth = width / bufferLength;
      let x = 0;

      for (let i = 0; i < bufferLength; i++) {
        const v = dataArray[i] / 128.0;
        const y = (v * height) / 2;

        if (i === 0) {
          canvasCtx.moveTo(x, y);
        } else {
          canvasCtx.lineTo(x, y);
        }
        x += sliceWidth;
      }

      canvasCtx.lineTo(width, height / 2);
      canvasCtx.stroke();
      canvasCtx.shadowBlur = 0; // Reset shadow for performance
    }

    function updateVolume(val) {
      document.getElementById('volValue').innerText = Math.round(val * 100) + '%';
      if (masterGainNode && audioCtx) {
        masterGainNode.gain.setValueAtTime(parseFloat(val), audioCtx.currentTime);
      }
    }

    function togglePlaySequence() {
      if (isPlaying) {
        stopSequence();
      } else {
        startSequencePlayback();
      }
    }

    function toggleLoop() {
      isLooping = !isLooping;
      const loopLabel = document.getElementById('loopLabel');
      const loopBtn = document.getElementById('loopBtn');
      if (isLooping) {
        loopLabel.innerText = "Endlosschleife: An";
        loopBtn.classList.add('border-cyan-500', 'text-cyan-400', 'bg-cyan-500/10');
      } else {
        loopLabel.innerText = "Endlosschleife: Aus";
        loopBtn.classList.remove('border-cyan-500', 'text-cyan-400', 'bg-cyan-500/10');
      }
    }

    function playSingleTone(index) {
      initAudioContext();
      const tone = sequence[index];
      if (!tone) return;

      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();

      osc.type = tone.type || 'sine';
      osc.frequency.setValueAtTime(tone.freq, audioCtx.currentTime);

      // Envelope to prevent click sounds
      const now = audioCtx.currentTime;
      const durationSec = tone.duration / 1000;
      gain.gain.setValueAtTime(0, now);
      gain.gain.linearRampToValueAtTime(1, now + 0.005);
      gain.gain.setValueAtTime(1, now + Math.max(0, durationSec - 0.005));
      gain.gain.linearRampToValueAtTime(0, now + durationSec);

      osc.connect(gain);
      gain.connect(masterGainNode);

      osc.start(now);
      osc.stop(now + durationSec);

      // Highlight single row briefly
      highlightRow(index);
      setTimeout(() => unhighlightRow(index), tone.duration);
    }

    function startSequencePlayback() {
      if (sequence.length === 0) {
        showToast('Keine Töne in der Sequenz vorhanden!', 'error');
        return;
      }

      initAudioContext();
      isPlaying = true;
      updateUIPlaybackState(true);

      let startTime = audioCtx.currentTime + 0.05; // Short delay
      let accumulatedDelayMs = 50;

      sequence.forEach((tone, idx) => {
        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();

        osc.type = tone.type || 'sine';
        osc.frequency.setValueAtTime(tone.freq, startTime);

        // Micro envelope for soft transition
        const durSec = tone.duration / 1000;
        gain.gain.setValueAtTime(0, startTime);
        gain.gain.linearRampToValueAtTime(1, startTime + 0.005);
        gain.gain.setValueAtTime(1, startTime + Math.max(0, durSec - 0.005));
        gain.gain.linearRampToValueAtTime(0, startTime + durSec);

        osc.connect(gain);
        gain.connect(masterGainNode);

        osc.start(startTime);
        osc.stop(startTime + durSec);
        activeOscillators.push(osc);

        // Schedule visual row highlight
        const currentDelay = accumulatedDelayMs;
        setTimeout(() => {
          if (isPlaying) {
            highlightRow(idx);
          }
        }, currentDelay);

        startTime += durSec;
        accumulatedDelayMs += tone.duration;
      });

      // Schedule completion / loop
      currentPlaybackTimeout = setTimeout(() => {
        unhighlightAllRows();
        if (isLooping && isPlaying) {
          startSequencePlayback();
        } else {
          stopSequence();
        }
      }, accumulatedDelayMs);
    }

    function stopSequence() {
      isPlaying = false;
      if (currentPlaybackTimeout) clearTimeout(currentPlaybackTimeout);
      activeOscillators.forEach(osc => {
        try { osc.stop(); } catch(e) {}
      });
      activeOscillators = [];
      unhighlightAllRows();
      updateUIPlaybackState(false);
    }

    function updateUIPlaybackState(playing) {
      const btnText = document.getElementById('playBtnText');
      const badge = document.getElementById('playingBadge');
      const btn = document.getElementById('playBtn');

      if (playing) {
        btnText.innerText = "Stoppen";
        badge.classList.remove('hidden');
        badge.classList.add('flex');
        btn.classList.remove('from-cyan-500', 'to-teal-500');
        btn.classList.add('from-red-500', 'to-pink-500', 'shadow-red-500/20');
      } else {
        btnText.innerText = "Sequenz Abspielen";
        badge.classList.add('hidden');
        badge.classList.remove('flex');
        btn.classList.remove('from-red-500', 'to-pink-500', 'shadow-red-500/20');
        btn.classList.add('from-cyan-500', 'to-teal-500');
      }
    }

    function renderSequenceTable() {
      const tbody = document.getElementById('sequenceTableBody');
      tbody.innerHTML = '';

      let totalDuration = 0;

      sequence.forEach((tone, index) => {
        totalDuration += Number(tone.duration);

        const tr = document.createElement('tr');
        tr.id = `seq-row-${index}`;
        tr.className = "hover:bg-gray-900/60 transition-colors group";

        tr.innerHTML = `
          <td class="py-2.5 px-3 text-center text-gray-500 font-bold">${index + 1}</td>
          <td class="py-2.5 px-3">
            <div class="flex items-center space-x-2">
              <input type="number" min="20" max="20000" value="${tone.freq}" 
                onchange="updateTone(${index}, 'freq', this.value)"
                class="w-20 bg-gray-900 border border-gray-700 text-cyan-300 rounded px-2 py-1 text-xs focus:outline-none focus:border-cyan-500">
              <span class="text-[10px] text-gray-500 hidden sm:inline">Hz</span>
            </div>
          </td>
          <td class="py-2.5 px-3">
            <div class="flex items-center space-x-2">
              <input type="number" min="10" max="10000" value="${tone.duration}" 
                onchange="updateTone(${index}, 'duration', this.value)"
                class="w-20 bg-gray-900 border border-gray-700 text-teal-300 rounded px-2 py-1 text-xs focus:outline-none focus:border-cyan-500">
              <span class="text-[10px] text-gray-500 hidden sm:inline">ms</span>
            </div>
          </td>
          <td class="py-2.5 px-3">
            <select onchange="updateTone(${index}, 'type', this.value)" 
              class="bg-gray-900 border border-gray-700 text-gray-300 text-xs rounded px-2 py-1 focus:outline-none focus:border-cyan-500">
              <option value="sine" ${tone.type === 'sine' ? 'selected' : ''}>Sinus</option>
              <option value="square" ${tone.type === 'square' ? 'selected' : ''}>Rechteck</option>
              <option value="sawtooth" ${tone.type === 'sawtooth' ? 'selected' : ''}>Sägezahn</option>
              <option value="triangle" ${tone.type === 'triangle' ? 'selected' : ''}>Dreieck</option>
            </select>
          </td>
          <td class="py-2.5 px-3 text-center">
            <button onclick="playSingleTone(${index})" title="Ton testen" class="p-1.5 bg-gray-800 hover:bg-cyan-500/20 hover:text-cyan-400 text-gray-400 rounded-lg transition-all">
              <i data-lucide="volume-2" class="w-3.5 h-3.5"></i>
            </button>
          </td>
          <td class="py-2.5 px-3 text-right">
            <div class="flex items-center justify-end space-x-1">
              <button onclick="moveRow(${index}, -1)" ${index === 0 ? 'disabled class="opacity-30 p-1"' : 'class="p-1 text-gray-400 hover:text-gray-200"'} title="Nach oben">
                <i data-lucide="chevron-up" class="w-3.5 h-3.5"></i>
              </button>
              <button onclick="moveRow(${index}, 1)" ${index === sequence.length - 1 ? 'disabled class="opacity-30 p-1"' : 'class="p-1 text-gray-400 hover:text-gray-200"'} title="Nach unten">
                <i data-lucide="chevron-down" class="w-3.5 h-3.5"></i>
              </button>
              <button onclick="duplicateRow(${index})" class="p-1 text-gray-400 hover:text-cyan-400" title="Duplizieren">
                <i data-lucide="copy" class="w-3.5 h-3.5"></i>
              </button>
              <button onclick="deleteRow(${index})" class="p-1 text-gray-400 hover:text-red-400" title="Löschen">
                <i data-lucide="trash-2" class="w-3.5 h-3.5"></i>
              </button>
            </div>
          </td>
        `;
        tbody.appendChild(tr);
      });

      // Update footers
      document.getElementById('totalTonesCount').innerText = sequence.length;
      document.getElementById('totalDurationCount').innerText = totalDuration + ' ms';

      lucide.createIcons();
      updateCodeOutput();
    }

    function addToneRow() {
      // Default to last tone or sensible default
      const last = sequence[sequence.length - 1] || { freq: 1000, duration: 100, type: 'sine' };
      sequence.push({ freq: last.freq + 200, duration: last.duration, type: last.type });
      renderSequenceTable();
    }

    function updateTone(index, field, value) {
      if (field === 'freq' || field === 'duration') {
        sequence[index][field] = Math.max(1, parseInt(value) || 0);
      } else {
        sequence[index][field] = value;
      }
      renderSequenceTable();
    }

    function deleteRow(index) {
      sequence.splice(index, 1);
      renderSequenceTable();
    }

    function duplicateRow(index) {
      const copy = { ...sequence[index] };
      sequence.splice(index + 1, 0, copy);
      renderSequenceTable();
    }

    function moveRow(index, direction) {
      const newIndex = index + direction;
      if (newIndex < 0 || newIndex >= sequence.length) return;
      const temp = sequence[index];
      sequence[index] = sequence[newIndex];
      sequence[newIndex] = temp;
      renderSequenceTable();
    }

    function clearSequence() {
      if (confirm('Möchtest du wirklich alle Töne aus der Sequenz entfernen?')) {
        sequence = [];
        renderSequenceTable();
      }
    }

    function loadPreset(key) {
      if (PRESETS[key]) {
        sequence = JSON.parse(JSON.stringify(PRESETS[key]));
        renderSequenceTable();
        showToast(`Preset "${key}" geladen.`, 'info');
      }
    }

    function highlightRow(index) {
      unhighlightAllRows();
      const row = document.getElementById(`seq-row-${index}`);
      if (row) {
        row.classList.add('active-row');
        row.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
      }
    }

    function unhighlightRow(index) {
      const row = document.getElementById(`seq-row-${index}`);
      if (row) row.classList.remove('active-row');
    }

    function unhighlightAllRows() {
      const rows = document.querySelectorAll('#sequenceTableBody tr');
      rows.forEach(r => r.classList.remove('active-row'));
    }

    function switchCodeTab(tab) {
      activeTab = tab;
      ['js', 'arduino', 'json'].forEach(t => {
        const btn = document.getElementById(`tab${t.charAt(0).toUpperCase() + t.slice(1)}`);
        if (t === tab) {
          btn.className = "px-2.5 py-1 rounded-md font-medium text-cyan-400 bg-gray-800 shadow";
        } else {
          btn.className = "px-2.5 py-1 rounded-md font-medium text-gray-400 hover:text-gray-200";
        }
      });
      updateCodeOutput();
    }

    function updateCodeOutput() {
      const codeOutput = document.getElementById('codeOutput');
      if (!codeOutput) return;

      if (activeTab === 'js') {
        let code = `// Web Audio API Sequenz-Wiedergabe\n`;
        code += `function playSineSequence() {\n`;
        code += `  const audioCtx = new (window.AudioContext || window.webkitAudioContext)();\n`;
        code += `  let currentTime = audioCtx.currentTime;\n\n`;
        
        sequence.forEach(t => {
          code += `  playSine(${t.freq}, ${t.duration}); // ${t.type}\n`;
        });

        code += `\n  function playSine(freq, durationMs) {\n`;
        code += `    const osc = audioCtx.createOscillator();\n`;
        code += `    const gain = audioCtx.createGain();\n`;
        code += `    osc.type = 'sine';\n`;
        code += `    osc.frequency.setValueAtTime(freq, currentTime);\n`;
        code += `    osc.connect(gain);\n`;
        code += `    gain.connect(audioCtx.destination);\n`;
        code += `    osc.start(currentTime);\n`;
        code += `    osc.stop(currentTime + durationMs / 1000);\n`;
        code += `    currentTime += durationMs / 1000;\n`;
        code += `  }\n`;
        code += `}\n\nplaySineSequence();`;

        codeOutput.textContent = code;
      } else if (activeTab === 'arduino') {
        let code = `// Arduino / ESP32 Ton-Sequenz\n`;
        code += `#define BUZZER_PIN 8\n\n`;
        code += `void playSequence() {\n`;
        sequence.forEach(t => {
          code += `  tone(BUZZER_PIN, ${t.freq}, ${t.duration});\n`;
          code += `  delay(${t.duration});\n`;
        });
        code += `}\n\nvoid setup() {\n  playSequence();\n}\n\nvoid loop() {}`;

        codeOutput.textContent = code;
      } else if (activeTab === 'json') {
        codeOutput.textContent = JSON.stringify(sequence, null, 2);
      }
    }

    function copyCodeToClipboard() {
      const codeText = document.getElementById('codeOutput').textContent;
      
      // Fallback for clipboard copy in restricted frames
      const tempTextArea = document.createElement('textarea');
      tempTextArea.value = codeText;
      document.body.appendChild(tempTextArea);
      tempTextArea.select();
      document.execCommand('copy');
      document.body.removeChild(tempTextArea);

      const btnText = document.getElementById('copyBtnText');
      btnText.innerText = 'Kopiert!';
      setTimeout(() => {
        btnText.innerText = 'Kopieren';
      }, 2000);

      showToast('Code in die Zwischenablage kopiert!', 'success');
    }

    function downloadJson() {
      const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(sequence, null, 2));
      const downloadAnchor = document.createElement('a');
      downloadAnchor.setAttribute("href", dataStr);
      downloadAnchor.setAttribute("download", "tone-sequence.json");
      document.body.appendChild(downloadAnchor);
      downloadAnchor.click();
      downloadAnchor.remove();
    }

    function importJson(event) {
      const file = event.target.files[0];
      if (!file) return;

      const reader = new FileReader();
      reader.onload = function(e) {
        try {
          const parsed = JSON.parse(e.target.result);
          if (Array.isArray(parsed)) {
            sequence = parsed.map(item => ({
              freq: Number(item.freq) || 440,
              duration: Number(item.duration) || 100,
              type: item.type || 'sine'
            }));
            renderSequenceTable();
            showToast('JSON-Sequenz erfolgreich geladen!', 'success');
          } else {
            showToast('Ungültiges JSON-Format!', 'error');
          }
        } catch (err) {
          showToast('Fehler beim Lesen der JSON-Datei', 'error');
        }
      };
      reader.readAsText(file);
    }

    function showToast(message, type = 'info') {
      const container = document.getElementById('toastContainer');
      const toast = document.createElement('div');

      const colorClasses = type === 'error' 
        ? 'bg-red-900/90 border-red-500 text-red-200' 
        : type === 'success' 
        ? 'bg-emerald-900/90 border-emerald-500 text-emerald-200' 
        : 'bg-cyan-900/90 border-cyan-500 text-cyan-200';

      toast.className = `pointer-events-auto border px-4 py-2.5 rounded-xl text-xs font-medium shadow-2xl flex items-center space-x-2 transition-all transform translate-y-2 opacity-0 ${colorClasses}`;
      toast.innerHTML = `<span>${message}</span>`;

      container.appendChild(toast);

      setTimeout(() => {
        toast.classList.remove('translate-y-2', 'opacity-0');
      }, 10);

      setTimeout(() => {
        toast.classList.add('opacity-0', 'translate-y-2');
        setTimeout(() => toast.remove(), 300);
      }, 3000);
    }
  </script>
</body>
</html>
)rawliteral";