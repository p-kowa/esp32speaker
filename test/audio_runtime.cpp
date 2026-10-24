#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using SemaphoreHandle_t = int;
constexpr int configTICK_RATE_HZ = 1000;
constexpr int portMAX_DELAY = -1;
constexpr int AUDIO_DATA = 1;
constexpr int CODEC_OGG = 1, CODEC_FLAC = 2, CODEC_OPUS = 3, CODEC_VORBIS = 4;
constexpr int CODEC_WAV = 5, CODEC_MP3 = 6, CODEC_AAC = 7, CODEC_M4A = 8, CODEC_NONE = 0;
constexpr int AUDIO_LOCALFILE = 2, ST_WEBFILE = 1, ST_NONE = 0;
void MP3Decoder_FreeBuffers() {}
void AACDecoder_FreeBuffers() {}
void FLACDecoder_FreeBuffers() {}
void OPUSDecoder_FreeBuffers() {}
void VORBISDecoder_FreeBuffers() {}
uint32_t clockTicks = 0;
std::function<void()> onLock;
int xSemaphoreCreateRecursiveMutex() { return 1; }
void vSemaphoreDelete(int) {}
void xSemaphoreTakeRecursive(int, int) {}
void xSemaphoreGiveRecursive(int) {}
void xSemaphoreTake(int, int) {
    if(onLock) {
        auto callback = onLock;
        onLock = {};
        callback();
    }
}
void xSemaphoreGive(int) {}
uint32_t millis() { return clockTicks++; }
void vTaskDelay(int ticks) { clockTicks += ticks; }
#define AUDIO_INFO(...) ((void)0)
#define log_i(...) ((void)0)
#define log_e(...) ((void)0)
#define min(a, b) ((a) < (b) ? (a) : (b))

#include "patched_audio_buffer.h"

void require(bool condition, const char* message) {
    if(!condition) throw std::runtime_error(message);
}

class TestBuffer : public AudioBuffer {
public:
    TestBuffer(size_t blockSize = 8, size_t bufferSize = 31) : AudioBuffer(blockSize) {
        m_buffer = static_cast<uint8_t*>(std::calloc(256, 1));
        m_buffSize = bufferSize;
        resetBuffer();
    }
    void write(const std::string& data) {
        for(unsigned char byte : data) {
            require(writeSpace() > 0, "buffer unexpectedly full");
            *getWritePtr() = byte;
            bytesWritten(1);
        }
    }
};

struct Client {
    std::string data;
    size_t position = 0;
    unsigned reads = 0;
    bool fragmented = false;
    uint32_t available() const { return static_cast<uint32_t>(data.size() - position); }
    int read() {
        if(fragmented && ++reads % 3 == 0) return -1;
        return position < data.size() ? static_cast<unsigned char>(data[position++]) : -1;
    }
    int16_t read(uint8_t* output, uint32_t count) {
        count = min(count, available());
        std::memcpy(output, data.data() + position, count);
        position += count;
        return static_cast<int16_t>(count);
    }
};

int eofCalls = 0;
void audio_eof_stream(const char*) { ++eofCalls; }
void audio_eof_speech(const char*) { ++eofCalls; }

class Audio {
public:
    Audio(bool wavTest = false) : InBuff(wavTest ? 32 : 8, wavTest ? 127 : 31) {}
    TestBuffer InBuff;
    Client client;
    Client* _client = &client;
    bool m_f_chunked = true, m_f_chunked_eof = false, m_f_firstCall = true;
    bool m_f_stream = false, m_f_metadata = false, m_f_running = true;
    bool m_f_tts = false;
    uint32_t m_contentlength = 0, m_audioDataSize = 0, m_audioDataStart = 0, m_t0 = 0;
    uint32_t m_chunkcount = 0;
    uint8_t m_controlCounter = 0;
    int m_streamType = ST_WEBFILE;
    int datamode = AUDIO_DATA;
    uint32_t m_metacount = 0, m_metaint = 0;
    int m_codec = 0, m_validSamples = 0, mutex_playAudioData = 1;
    int m_bytesNotDecoded = 0, stopCalls = 0, detectionCalls = 0, decodeCalls = 0;
    bool synced = false;
    const char* m_lastHost = "test";
    std::string decoded;
    void processWebStream();
    void processWebFile();
    int read_WAV_Header(uint8_t*, size_t);
    size_t readAudioHeader(uint32_t size) {
        const int result = read_WAV_Header(InBuff.getReadPtr(), size);
        return result > 0 ? result : 0;
    }
    uint32_t getFileSize() { return m_contentlength; }
    bool readID3V1Tag() { return false; }
    void setBitsPerSample(uint16_t) {}
    void setChannels(uint16_t) {}
    void setSampleRate(uint32_t) {}
    void setBitrate(uint32_t) {}
    size_t chunkedDataTransfer(uint8_t*);
    void performAudioTask();
    void playAudioData();
    int getDatamode() { return datamode; }
    uint32_t readMetadata(uint32_t, bool = false) { return 0; }
    bool streamDetection(uint32_t) { ++detectionCalls; return false; }
    uint8_t determineOggCodec(uint8_t*, uint16_t) { return 0; }
    void initializeDecoder() {}
    void stopSong() {
        require(!m_validSamples, "playback stopped before pending PCM samples were played");
        ++stopCalls;
        m_f_running = false;
    }
    void playChunk() { if(m_validSamples) --m_validSamples; }
    int sendBytes(uint8_t* data, size_t size) {
        ++decodeCalls;
        if(m_codec == CODEC_WAV) {
            decoded.append(reinterpret_cast<char*>(data), size);
            m_validSamples = 2;
            playChunk();
            return static_cast<int>(size);
        }
        if(!synced) { synced = true; return 0; }
        if(size < 4) return 0;
        decoded.append(reinterpret_cast<char*>(data), 4);
        m_validSamples = 2;
        playChunk();
        return 4;
    }
};

#include "patched_audio_methods.h"

void testBuffer() {
    TestBuffer buffer;
    buffer.write("abcdefgh");
    buffer.bytesWasRead(8);
    require(buffer.bufferFilled() == 0, "fully consumed buffer is incorrectly reported full");
    require(buffer.getMaxAvailableBytes() == 0, "empty buffer exposes stale bytes");
    require(buffer.freeSpace() == 31, "empty buffer has no free space");
    buffer.write("ijkl");
    require(buffer.bufferFilled() == 4, "refill after empty has wrong length");
    buffer.bytesWasRead(4);
    buffer.write(std::string(20, 'x'));
    buffer.bytesWasRead(19);
    require(*buffer.getReadPtr() == 'x', "read pointer wrapped one byte too early");
    require(buffer.bufferFilled() == 1, "last byte at ring boundary lost");
    buffer.bytesWasRead(1);
    require(buffer.bufferFilled() == 0, "wrapped empty buffer is incorrectly reported full");
    buffer.write(std::string(32, 'f'));
    require(buffer.bufferFilled() == 32, "full ring incorrectly reported empty");
    buffer.bytesWasRead(0);
    require(buffer.bufferFilled() == 32, "zero-byte read marked a full ring empty");
    buffer.bytesWasRead(32);
    require(buffer.bufferFilled() == 0, "full-ring drain replayed stale data");
}

void testParser() {
    for(const auto& input : {std::string("A\r\n"), std::string("\r\n0a;name=value\r\n")}) {
        Audio audio;
        audio.client.data = input;
        audio.client.fragmented = true;
        uint8_t bytes = 0;
        require(audio.chunkedDataTransfer(&bytes) == 10, "incorrect chunk size");
        require(bytes == input.size(), "chunk header byte count incorrect");
        require(!audio.m_f_chunked_eof && audio.m_f_running, "ordinary chunk marked as EOF");
    }
    for(const auto& input : {"0\r\n", "\r\n000;end=yes\r\n"}) {
        Audio audio;
        audio.client.data = input;
        uint8_t bytes = 0;
        require(audio.chunkedDataTransfer(&bytes) == 0 && audio.m_f_chunked_eof,
                "zero chunk did not terminate stream");
    }
    for(const auto& input : {"g\r\n", "10000000000000000\r\n", "\r\n"}) {
        Audio audio;
        audio.client.data = input;
        uint8_t bytes = 0;
        audio.chunkedDataTransfer(&bytes);
        require(!audio.m_f_running && !audio.m_f_chunked_eof, "bad header was accepted as EOF");
    }
}

void runStream(const std::string& wire, const std::string& expected) {
    Audio audio;
    audio.client.data = wire;
    audio.client.fragmented = true;
    const int previousEof = eofCalls;
    for(int iteration = 0; audio.m_f_running && iteration < 200; ++iteration) {
        audio.processWebStream();
        const int detections = audio.detectionCalls;
        if(audio.m_f_chunked_eof && audio.m_f_running) {
            audio.processWebStream();
            require(audio.detectionCalls == detections, "EOF ran reconnect detection");
        }
        audio.performAudioTask();
    }
    require(!audio.m_f_running, "finite stream never finished");
    require(audio.decoded == expected, "stream replayed or corrupted audio");
    require(audio.InBuff.bufferFilled() == 0, "stream left stale bytes");
    require(audio.stopCalls == 1 && eofCalls == previousEof + 1, "EOF was not delivered exactly once");
}

void testWorkerState() {
    Audio audio;
    audio.m_f_stream = true;
    audio.InBuff.write("abcdefgh");
    onLock = [&audio] { audio.m_f_stream = false; };
    audio.performAudioTask();
    require(audio.decodeCalls == 0, "worker decoded after EOF disabled playback");
    audio.m_f_stream = true;
    onLock = [&audio] { audio.m_f_running = false; };
    audio.performAudioTask();
    require(audio.decodeCalls == 0, "worker decoded after playback stopped");
}

void littleEndian(std::string& output, uint32_t value, int size) {
    for(int index = 0; index < size; ++index) output += static_cast<char>(value >> (8 * index));
}

std::string wav(const std::string& pcm, bool unknown = false) {
    std::string result = "RIFF";
    littleEndian(result, unknown ? UINT32_MAX : static_cast<uint32_t>(36 + pcm.size()), 4);
    result += "WAVEfmt ";
    littleEndian(result, 16, 4);
    littleEndian(result, 1, 2);
    littleEndian(result, 1, 2);
    littleEndian(result, 22050, 4);
    littleEndian(result, 44100, 4);
    littleEndian(result, 2, 2);
    littleEndian(result, 16, 2);
    result += "data";
    littleEndian(result, unknown ? UINT32_MAX : static_cast<uint32_t>(pcm.size()), 4);
    return result + pcm;
}

void testWav() {
    for(const size_t length : {0, 2, 6, 8, 10, 20, 100}) {
        for(const bool unknown : {false, true}) {
            Audio audio(true);
            audio.m_f_chunked = false;
            audio.m_codec = CODEC_WAV;
            const std::string pcm(length, 'p');
            audio.client.data = wav(pcm, unknown);
            audio.m_contentlength = static_cast<uint32_t>(audio.client.data.size());
            const int previousEof = eofCalls;
            for(int iteration = 0; audio.m_f_running && iteration < 500; ++iteration) {
                audio.processWebFile();
                audio.performAudioTask();
            }
            require(!audio.m_f_running, "short WAV never finished");
            if(audio.m_audioDataStart != 44) {
                std::cerr << "WAV length=" << length << " unknown=" << unknown
                          << " counter=" << static_cast<int>(audio.m_controlCounter)
                          << " offset=" << audio.m_audioDataStart << '\n';
            }
            require(audio.m_audioDataStart == 44, "wrong WAV data offset");
            require(audio.m_audioDataSize == length, "WAV data size includes/subtracts header");
            require(audio.decoded == pcm, "WAV lost or repeated PCM bytes");
            require(eofCalls == previousEof + 1, "WAV EOF not delivered exactly once");
        }
    }
    Audio truncated;
    truncated.m_f_chunked = false;
    truncated.m_codec = CODEC_WAV;
    truncated.client.data = "RIF";
    truncated.m_contentlength = 3;
    truncated.processWebFile();
    require(!truncated.m_f_running, "truncated WAV header did not stop");

    for(const int mode : {AUDIO_DATA, AUDIO_LOCALFILE}) {
        Audio audio(true);
        audio.datamode = mode;
        std::string file = wav("abcdefghijkl");
        // A legal JUNK chunk moves the data payload beyond the canonical 44 bytes.
        file.insert(36, std::string("JUNK\x04\0\0\0xxxx", 12));
        audio.m_contentlength = static_cast<uint32_t>(file.size());
        size_t offset = 0;
        for(int iteration = 0; audio.m_controlCounter != 100 && iteration < 100; ++iteration) {
            const int consumed = audio.read_WAV_Header(
                reinterpret_cast<uint8_t*>(&file[offset]), file.size() - offset);
            require(consumed >= 0, "WAV with extra chunk rejected");
            offset += consumed;
        }
        require(audio.m_audioDataStart == 56 && audio.m_audioDataSize == 12,
                "WAV extra chunk changed PCM payload length");
    }
}

int main(int argc, char** argv) {
    try {
        if(argc > 1 && std::string(argv[1]) == "--wav-only") {
            testWav();
            return 0;
        }
        testBuffer();
        if(argc > 1) return 0;
        testParser();
        runStream("3\r\nabc\r\n5;test=yes\r\ndefgh\r\n0\r\n\r\n", "abcdefgh");
        runStream("4\r\nabcd\r\n0\r\n\r\n", "abcd");
        runStream("0\r\n\r\n", "");
        runStream("3\r\nbad\r\n0\r\n\r\n", "");
        runStream("9\r\nabcdefghx\r\n0\r\n\r\n", "abcdefgh");
        runStream("10\r\n0123456789abcdef\r\n10\r\nghijklmnopqrstuv\r\n"
                  "10\r\nwxyz0123456789ab\r\n0\r\n\r\n",
                  "0123456789abcdefghijklmnopqrstuvwxyz0123456789ab");
        testWorkerState();
        testWav();
        std::cout << "Buffer, chunk parser, finite stream, worker and WAV regression tests passed\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
