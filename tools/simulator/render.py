"""Wandelt die vom Simulator geschriebenen frame_NNN.txt in PNGs um (nur Standardbibliothek)."""

import struct
import zlib
from pathlib import Path

HERE = Path(__file__).parent
BUILD = HERE / "build"
OUTPUT = HERE.parent.parent / "docs" / "design" / "renders"

SCALE = 4
BEZEL = 10
BEZEL_GRAY = 40

# (Dateiname, Variante, Position). Positionen zaehlen vom Ende der Bildfolge,
# damit die Laenge der Grafiktest-Animation egal ist:
#   -1 Optionen, -2 Dashboard, -3 Zusammenfassung, -4 WLAN, -5 Sensoren, -6 Akku,
#   -7 Endbild des Grafiktests. 0 ist das Startbild.
SCREENS = [
    ("01_startbild.png", "ok", 0),
    ("02_grafiktest.png", "ok", -7),
    ("03_akku_ok.png", "ok", -6),
    ("03_akku_fehler.png", "err", -6),
    ("04_sensoren_ok.png", "ok", -5),
    ("04_sensoren_fehler_luftfeuchte.png", "err", -5),
    ("04_sensoren_fehler_beide.png", "errall", -5),
    ("05_wlan_ok.png", "ok", -4),
    ("05_wlan_fehler.png", "err", -4),
    ("06_test_erfolgreich.png", "ok", -3),
    ("06_test_mit_fehlern.png", "err", -3),
    ("07_dashboard_ok.png", "ok", -2),
    ("07_dashboard_fehler.png", "err", -2),
    ("07_dashboard_alle_fehler.png", "errall", -2),
    ("08_optionen.png", "ok", -1),
]


def read_frame(path):
    return [[0 if c == "#" else 255 for c in row] for row in path.read_text().splitlines()]


def to_canvas(frame):
    width = len(frame[0]) * SCALE + 2 * BEZEL
    height = len(frame) * SCALE + 2 * BEZEL
    canvas = [[BEZEL_GRAY] * width for _ in range(height)]

    for y, row in enumerate(frame):
        line = []
        for value in row:
            line.extend([value] * SCALE)
        for dy in range(SCALE):
            canvas[BEZEL + y * SCALE + dy][BEZEL:BEZEL + len(line)] = line

    return canvas, width, height


def write_png(path, canvas, width, height):
    raw = b"".join(b"\x00" + bytes(row) for row in canvas)

    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")
    path.write_bytes(png)


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)

    for name, variant, position in SCREENS:
        frames = sorted((BUILD / variant).glob("frame_*.txt"))
        frame = read_frame(frames[position])
        canvas, width, height = to_canvas(frame)
        write_png(OUTPUT / name, canvas, width, height)
        print("geschrieben:", name)


if __name__ == "__main__":
    main()
