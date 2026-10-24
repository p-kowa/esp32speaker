"""Host-side patch tests. Run after PlatformIO has installed the DE dependency."""
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("patch_audio", ROOT / "scripts" / "patch_audio.py")
PATCH = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PATCH)


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


if __name__ == "__main__":
    unittest.main()
