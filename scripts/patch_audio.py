"""Apply the project's playback fixes to pinned ESP32-audioI2S 3.0.12."""
from pathlib import Path
from hashlib import sha256


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise RuntimeError("ESP32-audioI2S 3.0.12 patch context mismatch: " + old[:80])
    return text.replace(old, new, 1)


def patch_buffer_and_worker(source):
    old_buffer = """    if(m_readPtr >= m_endPtr) {
        size_t tmp = m_readPtr - m_endPtr;
        m_readPtr = m_buffer + tmp - 1;
    }
    xSemaphoreGiveRecursive(mutex_buffer);"""
    new_buffer = """    if(m_readPtr > m_endPtr) {
        size_t tmp = m_readPtr - m_endPtr;
        m_readPtr = m_buffer + tmp - 1;
    }
    // ESP32Speaker: a fully consumed ring is empty, not full.
    if(br && m_readPtr == m_writePtr) m_f_start = true;
    xSemaphoreGiveRecursive(mutex_buffer);"""
    old_worker = """    xSemaphoreTake(mutex_playAudioData, portMAX_DELAY);
    playAudioData();
    xSemaphoreGive(mutex_playAudioData);"""
    new_worker = """    xSemaphoreTake(mutex_playAudioData, portMAX_DELAY);
    // State may change while waiting for the EOF handler or a new connection.
    if(m_f_running && m_f_stream) playAudioData();
    xSemaphoreGive(mutex_playAudioData);"""
    if "// ESP32Speaker: a fully consumed ring" in source:
        if source.count(new_buffer) != 1 or source.count(new_worker) != 1:
            raise RuntimeError("Incomplete ESP32-audioI2S ring buffer patch")
        return source
    source = replace_once(source, old_buffer, new_buffer)
    return replace_once(source, old_worker, new_worker)


def patch_wav(source):
    changes = [
        ("    if(m_audioDataSize) availableBytes = min(m_audioDataSize - (byteCounter - m_audioDataStart), availableBytes);",
         """    if(m_audioDataSize && m_controlCounter == 100) {
        const uint32_t received = byteCounter >= m_audioDataStart ? byteCounter - m_audioDataStart : 0;
        availableBytes = min(received < m_audioDataSize ? m_audioDataSize - received : 0, availableBytes);
    }"""),
        ("    if(m_controlCounter == 0) {\n        m_controlCounter++;\n        if((*data != 'R')",
         """    // ESP32Speaker: bounded WAV headers and exact PCM length.
    const size_t required = m_controlCounter == 5 ? 16 :
                            m_controlCounter == 6 ? bts :
                            m_controlCounter >= 9 ? 0 : 4;
    if(len < required) return 0;
    if(m_controlCounter == 0) {
        m_controlCounter++;
        if((*data != 'R')"""),
        ("""        if(cs) { m_audioDataSize = cs - 44; }
        else { // sometimes there is nothing here
            if(getDatamode() == AUDIO_LOCALFILE) m_audioDataSize = getFileSize() - headerSize;
            if(m_streamType == ST_WEBFILE) m_audioDataSize = m_contentlength - headerSize;
        }""",
         """        // The data chunk size already excludes the WAV header.
        const size_t remaining = m_contentlength > headerSize ? m_contentlength - headerSize : 0;
        if(cs && cs != UINT32_MAX) m_audioDataSize = m_contentlength ? min(cs, remaining) : cs;
        else m_audioDataSize = remaining;"""),
        ("""    if(!m_f_stream && m_controlCounter == 100) {
        if((InBuff.freeSpace() > maxFrameSize) && (byteCounter < m_contentlength)) return;""",
         """    if(!m_f_stream && m_controlCounter == 100) {
        const bool wavComplete = m_codec == CODEC_WAV &&
            byteCounter >= m_audioDataStart && byteCounter - m_audioDataStart >= m_audioDataSize;
        if((InBuff.freeSpace() > maxFrameSize) && (byteCounter < m_contentlength) && !wavComplete) return;"""),
        ("""        if(InBuff.bufferFilled() > maxFrameSize) { // read the file header first
            int32_t bytesRead = readAudioHeader(InBuff.getMaxAvailableBytes());""",
         """        if(InBuff.bufferFilled() > maxFrameSize ||
           (m_codec == CODEC_WAV && byteCounter == m_contentlength)) {
            const uint8_t previousCounter = m_controlCounter;
            int32_t bytesRead = readAudioHeader(InBuff.getMaxAvailableBytes());
            if(m_codec == CODEC_WAV && byteCounter == m_contentlength &&
               bytesRead == 0 && previousCounter == m_controlCounter) {
                AUDIO_INFO("Incomplete WAV header");
                stopSong();
            }"""),
        ("""            if(InBuff.bufferFilled() > maxFrameSize) { // read the file header first
                InBuff.bytesWasRead(readAudioHeader(InBuff.getMaxAvailableBytes()));
            }""",
         """            if(InBuff.bufferFilled() > maxFrameSize ||
               (m_codec == CODEC_WAV && byteCounter == audiofile.size())) {
                const int32_t bytesRead = readAudioHeader(InBuff.getMaxAvailableBytes());
                if(bytesRead > 0) InBuff.bytesWasRead(bytesRead);
            }"""),
    ]
    tail = """        if(m_codec == CODEC_WAV) {
            xSemaphoreTake(mutex_playAudioData, portMAX_DELAY);
            if(!m_validSamples && InBuff.bufferFilled()) {
                const size_t remaining = InBuff.bufferFilled();
                int decoded = sendBytes(InBuff.getReadPtr(), remaining);
                if(decoded == 0 && !m_validSamples) decoded = sendBytes(InBuff.getReadPtr(), remaining);
                if(decoded > 0) InBuff.bytesWasRead(decoded);
                else if(!m_validSamples) {
                    AUDIO_INFO("Discarding incomplete PCM sample at WAV end");
                    InBuff.bytesWasRead(remaining);
                }
            }
            const bool drained = !InBuff.bufferFilled() && !m_validSamples;
            if(drained) m_f_stream = false;
            xSemaphoreGive(mutex_playAudioData);
            if(!drained) return;
        }
"""
    for condition in ("f_webFileDataComplete", "f_fileDataComplete"):
        anchor = f"    if({condition} && InBuff.bufferFilled() < InBuff.getMaxBlockSize()) {{\n"
        changes.append((anchor, anchor + tail))
    if "// ESP32Speaker: bounded WAV headers" in source:
        if any(source.count(new) != 1 for _, new in changes):
            raise RuntimeError("Incomplete ESP32-audioI2S WAV patch")
        return source
    for old, new in changes:
        source = replace_once(source, old, new)
    return source


def patched_sources(header, source):
    marker = "// ESP32Speaker: finite chunked streams"
    drain = """    // ESP32Speaker: finite chunked streams
    if(m_f_chunked_eof) {
        bool finished = false;
        xSemaphoreTake(mutex_playAudioData, portMAX_DELAY);
        m_f_stream = true;
        if(!m_validSamples && InBuff.bufferFilled() < maxFrameSize) {
            const size_t remaining = InBuff.bufferFilled();
            if(remaining) {
                int decoded = sendBytes(InBuff.getReadPtr(), remaining);
                // A zero return can establish sync without consuming a frame.
                if(decoded == 0 && !m_validSamples) decoded = sendBytes(InBuff.getReadPtr(), remaining);
                if(decoded > 0) InBuff.bytesWasRead(decoded);
                else if(!m_validSamples) {
                    AUDIO_INFO("Discarding undecodable tail of chunked stream");
                    InBuff.bytesWasRead(remaining);
                }
            }
            finished = !InBuff.bufferFilled() && !m_validSamples;
            if(finished) m_f_stream = false;
        }
        xSemaphoreGive(mutex_playAudioData);
        if(finished) {
            stopSong();
            AUDIO_INFO("End of chunked stream");
            if(audio_eof_stream) audio_eof_stream(m_lastHost);
        }
        return;
    }
"""
    parser = r"""    *bytes = 0;
    size_t chunksize = 0;
    bool digits = false;
    bool extension = false;
    const uint32_t start = millis();
    while(millis() - start < 2000) {
        const int b = _client->read();
        if(b < 0) { vTaskDelay(1); continue; }
        if(*bytes < 255) ++*bytes;
        if(b == '\r') continue;
        if(b == '\n') {
            if(!digits) continue; // CRLF after the preceding chunk is not a zero chunk.
            if(!chunksize) m_f_chunked_eof = true;
            return chunksize;
        }
        if(b == ';' && digits) { extension = true; continue; }
        if(extension) continue;
        int digit = -1;
        if(b >= '0' && b <= '9') digit = b - '0';
        else if(b >= 'a' && b <= 'f') digit = b - 'a' + 10;
        else if(b >= 'A' && b <= 'F') digit = b - 'A' + 10;
        if(digit < 0 || chunksize > (SIZE_MAX - digit) / 16) {
            AUDIO_INFO("Invalid HTTP chunk size");
            stopSong();
            return 0;
        }
        chunksize = chunksize * 16 + digit;
        digits = true;
    }
    AUDIO_INFO("HTTP chunk header timeout");
    stopSong();
    return 0;"""
    if marker in source:
        if (header.count("bool m_f_chunked_eof = false;") != 1
                or source.count(drain) != 1 or source.count(parser) != 1
                or source.count("    m_f_chunked_eof = false;") != 1
                or source.count("if(m_f_chunked && !m_f_chunked_eof && availableBytes)") != 1):
            raise RuntimeError("Incomplete ESP32-audioI2S chunked EOF patch")
        return header, patch_wav(patch_buffer_and_worker(source))

    if (sha256(header.encode("utf-8")).hexdigest() !=
            "b6b946848804225fa5043558764ee9fb87097cb721c83f46261aedb44a92ef4e"
            or sha256(source.encode("utf-8")).hexdigest() !=
            "ffde34db30d3fbae7a9ac45b8b9fb94d6b89cdd06192a811cd3e57138cd2afbf"):
        raise RuntimeError("ESP32-audioI2S 3.0.12 patch context mismatch: unexpected library sources")

    # Insert before a method declaration so formatting of member fields is irrelevant.
    header = replace_once(header, "  void            processWebStream();",
                          "    bool m_f_chunked_eof = false;\n  void            processWebStream();")
    source = replace_once(source, "    m_f_chunked = false; // Assume not chunked",
                          "    m_f_chunked_eof = false;\n    m_f_chunked = false; // Assume not chunked")
    source = replace_once(source,
                          "    if(m_f_chunked && availableBytes) {\n        uint8_t readedBytes = 0;",
                          "    if(m_f_chunked && !m_f_chunked_eof && availableBytes) {\n        uint8_t readedBytes = 0;")
    source = replace_once(source,
                          "    // we have metadata  -",
                          drain + "    // we have metadata  -")
    start = source.index("    uint8_t  byteCounter = 0;", source.index("size_t Audio::chunkedDataTransfer("))
    end = source.index("\n}\n", start)
    source = source[:start] + parser + source[end:]
    return header, patch_wav(patch_buffer_and_worker(source))


def apply_patch(directory):
    header_path = directory / "Audio.h"
    source_path = directory / "Audio.cpp"
    header = header_path.read_text(encoding="utf-8")
    source = source_path.read_text(encoding="utf-8")
    new_header, new_source = patched_sources(header, source)
    if new_header != header or new_source != source:
        header_path.write_text(new_header, encoding="utf-8")
        source_path.write_text(new_source, encoding="utf-8")
        print("Applied ESP32-audioI2S playback fixes")


if __name__ == "__main__":
    import sys
    apply_patch(Path(sys.argv[1]))
elif "Import" in globals():
    Import("env")
    apply_patch(Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV") /
                "ESP32-audioI2S" / "src")
