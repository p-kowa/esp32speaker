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
        <button class="btn-danger" onclick="stopAudio()">⏹ Stop</button>
      </div>
    </div>

    <div class="card">
      <h3 style="font-size: 1rem; margin-bottom: 0.75rem; color: var(--accent);">Favoriten</h3>
      <div class="station-grid" id="presetList"></div>
      <button type="button" class="btn-primary alarm-station-button" id="saveRadioAlarm">⏰ Als Weckton speichern</button>

      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">Senderliste</h3>
      <input type="search" id="stationSearch" class="station-search" placeholder="Sender suchen...">
      <div id="stationList"></div>
      <div class="pager">
        <button type="button" id="stationPrev">◀</button>
        <span id="stationPageInfo">--</span>
        <button type="button" id="stationNext">▶</button>
      </div>
      <div id="stationMsg" class="hint" style="margin-top: 0.5rem;"></div>

      <h3 style="font-size: 0.9rem; margin-top: 1rem; margin-bottom: 0.4rem; color: var(--text-muted);">Eigene Stream-URL</h3>
      <div class="custom-url-box">
        <input type="text" id="customUrl" placeholder="http://.../stream.mp3">
        <button class="btn-primary" onclick="playCustomUrl()">Play</button>
      </div>
    </div>

    <div class="card advanced-card" id="advancedCard">
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
    let stationPage = null;
    let stationOffset = 0;
    let stationQuery = '';
    const STATION_PAGE_SIZE = 20;

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

    function stopAudio() {
      fetch('/api/stop');
      setTimeout(updateStatus, 200);
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

      <a href="/" class="nav-link">← Zurück zum Radio</a>
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

      <a href="/" class="nav-link">← Zurück zum Radio</a>
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

