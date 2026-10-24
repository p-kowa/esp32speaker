"""Host-side patch tests. Run after PlatformIO has installed the DE dependency."""
import importlib.util
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("patch_audio", ROOT / "scripts" / "patch_audio.py")
PATCH = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PATCH)


def function_source(source, signature):
    start = source.index(signature)
    end = source.index("\n}\n", start) + 3
    return source[start:end]


class PatchAudioTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        library = ROOT / ".pio" / "libdeps" / "esp32-s3-devkitc-1_DE" / "ESP32-audioI2S"

        def original(name):
            return subprocess.check_output(
                ["git", "-C", str(library), "show", "HEAD:src/" + name],
            ).decode("utf-8").replace("\r\n", "\n")

        cls.header = original("Audio.h")
        cls.source = original("Audio.cpp")

    def test_apply_and_repeat(self):
        header, source = PATCH.patched_sources(self.header, self.source)
        self.assertEqual((header, source), PATCH.patched_sources(header, source))
        self.assertEqual(header.count("bool m_f_chunked_eof = false;"), 1)
        self.assertEqual(source.count("// ESP32Speaker: finite chunked streams"), 1)

    def test_generated_parser_uses_single_character_crlf(self):
        _, source = PATCH.patched_sources(self.header, self.source)
        parser = source.split("size_t Audio::chunkedDataTransfer(", 1)[1].split("\n}\n", 1)[0]
        self.assertIn(r"if(b == '\r')", parser)
        self.assertIn(r"if(b == '\n')", parser)
        self.assertNotIn(r"'\\r'", parser)
        self.assertNotIn(r"'\\n'", parser)
        self.assertLess(parser.index("if(!digits) continue;"),
                        parser.index("if(!chunksize) m_f_chunked_eof = true;"))
        self.assertIn("if(extension) continue;", parser)
        self.assertIn("(SIZE_MAX - digit) / 16", parser)

    def test_eof_drains_before_reconnect_detection(self):
        _, source = PATCH.patched_sources(self.header, self.source)
        stream = source.split("void Audio::processWebStream() {", 1)[1].split("\n}\n", 1)[0]
        self.assertLess(stream.index("if(m_f_chunked_eof)"), stream.index("streamDetection("))
        self.assertIn("!m_validSamples && InBuff.bufferFilled() < maxFrameSize", stream)
        self.assertIn("finished = !InBuff.bufferFilled() && !m_validSamples;", stream)
        self.assertIn("if(decoded == 0 && !m_validSamples) decoded = sendBytes(", stream)
        self.assertLess(stream.index("xSemaphoreGive(mutex_playAudioData)"), stream.index("stopSong();"))
        self.assertIn("if(audio_eof_stream) audio_eof_stream(m_lastHost);", stream)

    def test_rejects_changed_original(self):
        for header, source in ((self.header + "\n", self.source),
                               (self.header, self.source.replace("return chunksize;", "return 0;"))):
            with self.subTest(header_changed=header != self.header):
                with self.assertRaisesRegex(RuntimeError, "context mismatch"):
                    PATCH.patched_sources(header, source)

    def test_rejects_partial_patch(self):
        header, source = PATCH.patched_sources(self.header, self.source)
        mutations = [
            (self.header, source),
            (header, source.replace("    m_f_chunked_eof = false;", "")),
            (header, source.replace("if(!digits) continue;", "if(!digits) return 0;")),
            (header, source.replace("if(finished) m_f_stream = false;", "")),
        ]
        for changed_header, changed_source in mutations:
            with self.subTest(source=changed_source[-80:]):
                with self.assertRaisesRegex(RuntimeError, "Incomplete"):
                    PATCH.patched_sources(changed_header, changed_source)

    def test_files_are_only_written_after_validation(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            header_path = directory / "Audio.h"
            source_path = directory / "Audio.cpp"
            header_path.write_text(self.header, encoding="utf-8")
            source_path.write_text("unsupported source", encoding="utf-8")
            with self.assertRaises(RuntimeError):
                PATCH.apply_patch(directory)
            self.assertEqual(header_path.read_text(encoding="utf-8"), self.header)
            self.assertEqual(source_path.read_text(encoding="utf-8"), "unsupported source")
            source_path.write_text(self.source, encoding="utf-8")
            PATCH.apply_patch(directory)
            before = (header_path.read_bytes(), source_path.read_bytes())
            PATCH.apply_patch(directory)
            self.assertEqual(before, (header_path.read_bytes(), source_path.read_bytes()))

    def test_upgrades_previous_patch_without_changing_header(self):
        header, source = PATCH.patched_sources(self.header, self.source)
        previous = source
        for signature in ("void AudioBuffer::bytesWasRead(", "void Audio::performAudioTask("):
            previous = previous.replace(function_source(source, signature),
                                        function_source(self.source, signature))
        self.assertEqual((header, source), PATCH.patched_sources(header, previous))
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "Audio.h").write_text(header, encoding="utf-8")
            (directory / "Audio.cpp").write_text(previous, encoding="utf-8")
            PATCH.apply_patch(directory)
            self.assertEqual((directory / "Audio.h").read_text(encoding="utf-8"), header)
            self.assertEqual((directory / "Audio.cpp").read_text(encoding="utf-8"), source)

    def test_rejects_partial_buffer_patch(self):
        header, source = PATCH.patched_sources(self.header, self.source)
        for fragment in ("if(br && m_readPtr == m_writePtr) m_f_start = true;",
                         "if(m_f_running && m_f_stream) playAudioData();"):
            with self.subTest(fragment=fragment):
                with self.assertRaisesRegex(RuntimeError, "Incomplete"):
                    PATCH.patched_sources(header, source.replace(fragment, ""))

    def test_upgrades_existing_chunked_patch_with_wav_fix(self):
        header, source = PATCH.patched_sources(self.header, self.source)
        previous = source
        for signature in ("int Audio::read_WAV_Header(", "void Audio::processWebFile(",
                          "void Audio::processLocalFile("):
            previous = previous.replace(function_source(source, signature),
                                        function_source(self.source, signature))
        self.assertEqual((header, source), PATCH.patched_sources(header, previous))
        with self.assertRaisesRegex(RuntimeError, "Incomplete.*WAV"):
            PATCH.patched_sources(header, source.replace(
                "else m_audioDataSize = remaining;", "else m_audioDataSize = 0;"))

    def test_native_runtime(self):
        compiler = next((shutil.which(name) for name in ("cl", "g++", "clang++")
                         if shutil.which(name)), None)
        if compiler is None:
            self.skipTest("Native C++ compiler unavailable; use a Visual Studio developer shell")

        def buffer_code(header, source):
            start = header.index("class AudioBuffer {")
            end = header.index("\n};", start) + 3
            methods = [
                "AudioBuffer::AudioBuffer(", "AudioBuffer::~AudioBuffer(",
                "uint16_t AudioBuffer::getMaxBlockSize(",
                "size_t AudioBuffer::freeSpace(", "size_t AudioBuffer::writeSpace(",
                "size_t AudioBuffer::bufferFilled(", "size_t AudioBuffer::getMaxAvailableBytes(",
                "void AudioBuffer::bytesWritten(", "void AudioBuffer::bytesWasRead(",
                "uint8_t* AudioBuffer::getWritePtr(", "uint8_t* AudioBuffer::getReadPtr(",
                "void AudioBuffer::resetBuffer(",
            ]
            # Some library methods are single-line definitions.
            result = header[start:end] + "\n"
            for method in methods:
                position = source.index(method)
                line_end = source.index("\n", position)
                if "}" in source[position:line_end]:
                    result += source[position:line_end] + "\n"
                else:
                    result += function_source(source, method)
            return result

        header, source = PATCH.patched_sources(self.header, self.source)
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "patched_audio_methods.h").write_text("".join(
                function_source(source, signature) for signature in [
                    "void Audio::processWebStream(",
                    "size_t Audio::chunkedDataTransfer(",
                    "void Audio::performAudioTask(",
                    "void Audio::playAudioData(",
                    "int Audio::read_WAV_Header(",
                    "void Audio::processWebFile(",
                ]
            ), encoding="utf-8")
            harness = ROOT / "test" / "audio_runtime.cpp"
            executable = directory / "audio_runtime.exe"

            def compile_buffer(buffer_header, buffer_source):
                (directory / "patched_audio_buffer.h").write_text(
                    buffer_code(buffer_header, buffer_source), encoding="utf-8")
                if Path(compiler).stem.lower() == "cl":
                    command = [compiler, "/nologo", "/EHsc", "/std:c++17",
                               "/I" + str(directory), str(harness), "/Fe:" + str(executable)]
                else:
                    command = [compiler, "-std=c++17", "-I", str(directory),
                               str(harness), "-o", str(executable)]
                result = subprocess.run(command, cwd=directory, capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

            compile_buffer(self.header, self.source)
            baseline = subprocess.run([str(executable), "--buffer-only"],
                                      capture_output=True, text=True)
            self.assertNotEqual(baseline.returncode, 0)
            self.assertIn("fully consumed buffer is incorrectly reported full", baseline.stderr)
            compile_buffer(header, source)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())
            methods_path = directory / "patched_audio_methods.h"
            methods = methods_path.read_text(encoding="utf-8")
            for signature in ("int Audio::read_WAV_Header(", "void Audio::processWebFile("):
                methods = methods.replace(function_source(source, signature),
                                          function_source(self.source, signature))
            methods_path.write_text(methods, encoding="utf-8")
            compile_buffer(header, source)
            baseline_wav = subprocess.run([str(executable), "--wav-only"],
                                          capture_output=True, text=True, timeout=10)
            self.assertNotEqual(baseline_wav.returncode, 0,
                                "WAV regression must fail without the WAV patch")


if __name__ == "__main__":
    unittest.main()
