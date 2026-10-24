"""Apply the project's chunked-stream EOF fix to pinned ESP32-audioI2S 3.0.12."""
from pathlib import Path
from hashlib import sha256


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
        return header, source

    if (sha256(header.encode("utf-8")).hexdigest() !=
            "b6b946848804225fa5043558764ee9fb87097cb721c83f46261aedb44a92ef4e"
            or sha256(source.encode("utf-8")).hexdigest() !=
            "ffde34db30d3fbae7a9ac45b8b9fb94d6b89cdd06192a811cd3e57138cd2afbf"):
        raise RuntimeError("ESP32-audioI2S 3.0.12 patch context mismatch: unexpected library sources")

    def replace_once(text, old, new):
        if text.count(old) != 1:
            raise RuntimeError("ESP32-audioI2S 3.0.12 patch context mismatch: " + old[:80])
        return text.replace(old, new, 1)

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
    return header, source


def apply_patch(directory):
    header_path = directory / "Audio.h"
    source_path = directory / "Audio.cpp"
    header = header_path.read_text(encoding="utf-8")
    source = source_path.read_text(encoding="utf-8")
    new_header, new_source = patched_sources(header, source)
    if new_header != header:
        header_path.write_text(new_header, encoding="utf-8")
        source_path.write_text(new_source, encoding="utf-8")
        print("Applied ESP32-audioI2S chunked EOF patch")


if __name__ == "__main__":
    import sys
    apply_patch(Path(sys.argv[1]))
elif "Import" in globals():
    Import("env")
    apply_patch(Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV") /
                "ESP32-audioI2S" / "src")
